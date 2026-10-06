#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <mutex>

namespace NeoWPF::UI::Properties {

namespace {
std::mutex g_anyEqualityComparersMutex;
std::unordered_map<std::type_index, AnyEqualityComparer> g_anyEqualityComparers;
}

void RegisterAnyEqualityComparer(const std::type_index& valueType,
                                 AnyEqualityComparer comparer) noexcept
{
    if (!comparer) {
        return;
    }

    try {
        const std::lock_guard lock(g_anyEqualityComparersMutex);
        g_anyEqualityComparers.insert_or_assign(valueType, comparer);
    } catch (...) {
    }
}

bool ValuesEqual(const std::any& left, const std::any& right) noexcept
{
    if (left.has_value() != right.has_value()) {
        return false;
    }

    if (!left.has_value()) {
        return true;
    }

    if (left.type() != right.type()) {
        return false;
    }

    if (left.type() == typeid(bool)) {
        return std::any_cast<bool>(left) == std::any_cast<bool>(right);
    }
    if (left.type() == typeid(int)) {
        return std::any_cast<int>(left) == std::any_cast<int>(right);
    }
    if (left.type() == typeid(float)) {
        return std::any_cast<float>(left) == std::any_cast<float>(right);
    }
    if (left.type() == typeid(double)) {
        return std::any_cast<double>(left) == std::any_cast<double>(right);
    }
    if (left.type() == typeid(std::string)) {
        return std::any_cast<std::string>(left) == std::any_cast<std::string>(right);
    }
    if (left.type() == typeid(std::wstring)) {
        return std::any_cast<std::wstring>(left) == std::any_cast<std::wstring>(right);
    }

    try {
        AnyEqualityComparer comparer = nullptr;
        {
            const std::lock_guard lock(g_anyEqualityComparersMutex);
            const auto it = g_anyEqualityComparers.find(std::type_index(left.type()));
            if (it != g_anyEqualityComparers.end()) {
                comparer = it->second;
            }
        }
        if (comparer) {
            return comparer(left, right);
        }
    } catch (...) {
    }

    return false;
}

bool ValuesEqual(const DependencyObject& owner,
                 const DependencyProperty& property,
                 const std::any& expectedValue)
{
    return ValuesEqual(owner.GetRawValue(property), expectedValue);
}

DependencyProperty::DependencyProperty(const std::string& name,
                                       const std::type_index& propertyType,
                                       const std::type_index& ownerType,
                                       const PropertyMetadata& metadata,
                                       const std::type_index& attachedTargetType,
                                       bool isAttached)
    : m_propertyType(propertyType)
    , m_ownerType(ownerType)
    , m_name(name)
    , m_metadata(metadata)
    , m_attachedTargetType(attachedTargetType)
    , m_isAttached(isAttached)
{
}

const std::type_index& DependencyProperty::GetPropertyType() const noexcept {
    return m_propertyType;
}

const std::type_index& DependencyProperty::GetOwnerType() const noexcept {
    return m_ownerType;
}

const PropertyMetadata& DependencyProperty::GetMetadata() const noexcept {
    return m_metadata;
}

const std::string& DependencyProperty::GetName() const noexcept {
    return m_name;
}

bool DependencyProperty::IsAttached() const noexcept {
    return m_isAttached;
}

const std::type_index& DependencyProperty::GetAttachedTargetType() const noexcept {
    return m_attachedTargetType;
}

const NeoWPF::UI::Markup::Xaml::XamlPropertyBinding* DependencyProperty::GetXamlBinding() const noexcept {
    return NeoWPF::UI::Markup::Xaml::XamlPropertyBindingRegistry::Get().Find(*this);
}

bool DependencyProperty::TryParseValue(const std::string& rawValue, std::any& parsedValue) const
{
    if (m_propertyType == typeid(std::any) && !m_metadata.Converter) {
        parsedValue = rawValue;
        return true;
    } /////////////////////////

    if (m_metadata.Converter) {
        return m_metadata.Converter->ConvertFromString(rawValue, parsedValue);
    }

    if (m_metadata.ValueParser) {
        return m_metadata.ValueParser(rawValue, parsedValue);
    }

    return false;
}

bool DependencyProperty::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    expression.clear();
    return m_metadata.Converter && m_metadata.Converter->TryConvertToCode(rawValue, expression);
}

DependencyPropertyRegistry& DependencyPropertyRegistry::Get() {
    static DependencyPropertyRegistry registry;
    return registry;
}

const DependencyProperty& DependencyPropertyRegistry::RegisterProperty(const std::string& name,
                                                                       const std::type_index& propertyType,
                                                                       const std::type_index& ownerType,
                                                                       const PropertyMetadata& metadata)
{
    for (const auto& existingProperty : m_properties) {
        if (existingProperty->GetName() == name && existingProperty->GetOwnerType() == ownerType) {
            return *existingProperty;
        }
    }

    auto property = std::make_unique<DependencyProperty>(name, propertyType, ownerType, metadata);
    const DependencyProperty& ref = *property;
    m_properties.push_back(std::move(property));
    return ref;
}

const DependencyProperty* DependencyPropertyRegistry::FindProperty(const std::string& name,
                                                                   const std::type_index& ownerType) const
{
    for (const auto& property : m_properties) {
        if (property->GetName() == name && property->GetOwnerType() == ownerType) {
            return property.get();
        }
    }
    return nullptr;
}

const DependencyProperty* DependencyPropertyRegistry::FindProperty(const std::string& name,
                                                                   const DependencyObject& ownerInstance) const
{
    const std::type_index ownerType(typeid(ownerInstance));

    const auto splitQualifiedName = [](const std::string& input) {
        const auto dot = input.find('.');
        if (dot == std::string::npos) {
            return std::make_pair(std::string(), input);
        }
        return std::make_pair(input.substr(0, dot), input.substr(dot + 1));
    };

    const auto [ownerQualifier, propertyName] = splitQualifiedName(name);
    const auto matchesName = [&](const auto& property) {
        return property->GetName() == name || property->GetName() == propertyName;
    };

    for (const auto& property : m_properties) {
        if (!matchesName(property)) {
            continue;
        }
        if (property->GetOwnerType() == ownerType) {
            return property.get();
        }
    }

    for (const auto& property : m_properties) {
        if (!matchesName(property)) {
            continue;
        }

        const auto it = m_ownerTypeChecks.find(property->GetOwnerType());
        if (it != m_ownerTypeChecks.end() && it->second(&ownerInstance)) {
            return property.get();
        }
    }

    if (!ownerQualifier.empty()) {
        for (const auto& property : m_properties) {
            if (!property->IsAttached() || property->GetName() != propertyName) {
                continue;
            }
            const auto targetType = property->GetAttachedTargetType();
            if (targetType == ownerType) {
                return property.get();
            }
            const auto targetCheck = m_ownerTypeChecks.find(targetType);
            if (targetCheck != m_ownerTypeChecks.end() && targetCheck->second(&ownerInstance)) {
                return property.get();
            }
        }
    }

    return nullptr;
}

std::vector<const DependencyProperty*> DependencyPropertyRegistry::GetPropertiesFor(const DependencyObject& ownerInstance) const
{
    const std::type_index ownerType(typeid(ownerInstance));
    std::vector<const DependencyProperty*> result;

    for (const auto& property : m_properties) {
        if (property->GetOwnerType() == ownerType) {
            result.push_back(property.get());
            continue;
        }

        const auto it = m_ownerTypeChecks.find(property->GetOwnerType());
        if (it != m_ownerTypeChecks.end() && it->second(&ownerInstance)) {
            result.push_back(property.get());
        }
    }

    return result;
}

const DependencyProperty& DependencyPropertyRegistry::RegisterAttachedProperty(const std::string& name,
                                                                               const std::type_index& propertyType,
                                                                               const std::type_index& ownerType,
                                                                               const std::type_index& attachedTargetType,
                                                                               const PropertyMetadata& metadata)
{
    for (const auto& existingProperty : m_properties) {
        if (existingProperty->GetName() == name &&
            existingProperty->GetOwnerType() == ownerType &&
            existingProperty->IsAttached() &&
            existingProperty->GetAttachedTargetType() == attachedTargetType) {
            return *existingProperty;
        }
    }

    auto property = std::make_unique<DependencyProperty>(name, propertyType, ownerType, metadata, attachedTargetType, true);
    const DependencyProperty& ref = *property;
    m_properties.push_back(std::move(property));
    return ref;
}

const DependencyProperty* DependencyPropertyRegistry::FindAttachedProperty(const std::string& name,
                                                                          const std::type_index& targetType,
                                                                          const std::type_index& ownerType) const
{
    for (const auto& property : m_properties) {
        if (property->GetName() == name &&
            property->GetOwnerType() == ownerType &&
            property->IsAttached() &&
            property->GetAttachedTargetType() == targetType) {
            return property.get();
        }
    }
    return nullptr;
}

const DependencyProperty* DependencyPropertyRegistry::FindAttachedProperty(const std::string& name,
                                                                          const DependencyObject& targetInstance,
                                                                          const std::type_index& ownerType) const
{
    const std::type_index targetType(typeid(targetInstance));
    return FindAttachedProperty(name, targetType, ownerType);
}

}