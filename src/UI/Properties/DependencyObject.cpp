#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Data/BindingExpression.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Localization/LocaleExpression.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <stdexcept>

namespace NeoWPF::UI::Properties {

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

void DependencyObject::SetValueInternal(const DependencyProperty& property,
                                       std::any value,
                                       PropertyValueSource source)
{
    if (m_isApplyingPropertyValue) {
        return;
    }

    std::any oldValue = GetRawValue(property);

    if (source == PropertyValueSource::Expression) {
        m_localValues.erase(&property);
    } else if (source != PropertyValueSource::Local &&
               source != PropertyValueSource::Animation &&
               source != PropertyValueSource::Trigger &&
               source != PropertyValueSource::Style) {
        ClearBindingExpression(property);
    }

    if (source == PropertyValueSource::Animation) {
        m_animatedValues[&property] = std::move(value);
    } else if (source == PropertyValueSource::Trigger) {
        m_triggerValues[&property] = std::move(value);
    } else if (source == PropertyValueSource::Style) {
        m_styleValues[&property] = std::move(value);
    } else if (source == PropertyValueSource::Local) {
        ClearLocaleExpression(property);
        m_localValues[&property] = std::move(value);
    } else if (source == PropertyValueSource::Expression) {
        m_expressionValues[&property] = std::move(value);
    } else {
        m_localValues[&property] = std::move(value);
    }

    const std::any newValue = GetRawValue(property);
    NotifyDependencyPropertyChanged(property, oldValue, newValue, source);
}

void DependencyObject::SetBindingExpression(const DependencyProperty& property,
                                           std::shared_ptr<::NeoWPF::UI::Data::BindingExpression> expression)
{
    if (auto currentExpression = GetBindingExpression(property)) {
        currentExpression->Detach();
    }
    m_bindingExpressions[&property] = std::move(expression);
}

std::shared_ptr<::NeoWPF::UI::Data::BindingExpression> DependencyObject::GetBindingExpression(const DependencyProperty& property) const
{
    if (auto it = m_bindingExpressions.find(&property); it != m_bindingExpressions.end()) {
        return it->second;
    }
    return nullptr;
}

void DependencyObject::ClearBindingExpression(const DependencyProperty& property)
{
    m_bindingExpressions.erase(&property);
    m_expressionValues.erase(&property);
    m_localValues.erase(&property);
}

void DependencyObject::SetLocaleExpression(
    const DependencyProperty& property,
    std::shared_ptr<::NeoWPF::UI::Localization::LocaleExpression> expression)
{
    ClearLocaleExpression(property);
    m_localeExpressions[&property] = std::move(expression);
}

std::shared_ptr<::NeoWPF::UI::Localization::LocaleExpression>
DependencyObject::GetLocaleExpression(const DependencyProperty& property) const
{
    if (auto it = m_localeExpressions.find(&property); it != m_localeExpressions.end()) {
        return it->second;
    }
    return nullptr;
}

void DependencyObject::ClearLocaleExpression(const DependencyProperty& property)
{
    if (auto it = m_localeExpressions.find(&property); it != m_localeExpressions.end()) {
        if (it->second) {
            it->second->Detach();
        }
        m_localeExpressions.erase(it);
    }
    m_expressionValues.erase(&property);
}

void DependencyObject::ClearLocaleExpressionValue(const DependencyProperty& property)
{
    const std::any oldValue = GetRawValue(property);
    const bool hadExpression = m_localeExpressions.find(&property) != m_localeExpressions.end() ||
                               m_expressionValues.find(&property) != m_expressionValues.end();
    ClearLocaleExpression(property);
    if (hadExpression) {
        NotifyDependencyPropertyChanged(property,
                                        oldValue,
                                        GetRawValue(property),
                                        PropertyValueSource::Expression);
    }
}

void DependencyObject::NotifyDependencyPropertyChanged(const DependencyProperty& property,
                                                       const std::any& oldValue,
                                                       const std::any& newValue,
                                                       PropertyValueSource source)
{
    const auto& metadata = property.GetMetadata();
    const bool previousApplying = m_isApplyingPropertyValue;
    m_isApplyingPropertyValue = true;
    if (metadata.PropertyChangedCallback) {
        DependencyPropertyChangedEventArgs args{&property, oldValue, newValue, source};
        metadata.PropertyChangedCallback(this, args);
    }
    if (auto handlers = m_dependencyPropertyChangedHandlers.find(&property); handlers != m_dependencyPropertyChangedHandlers.end()) {
        for (const auto& [id, handler] : handlers->second) {
            (void)id;
            if (handler) {
                handler(oldValue, newValue);
            }
        }
    }
    m_isApplyingPropertyValue = previousApplying;
    OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);
}

void DependencyObject::RefreshBindingExpressions()
{
    for (auto& [property, expression] : m_bindingExpressions) {
        if (expression) {
            expression->Refresh();
        }
    }
}

std::size_t DependencyObject::AddDependencyPropertyChangedHandler(const DependencyProperty& property,
                                                                std::function<void(const std::any&, const std::any&)> handler)
{
    const auto handlerId = m_nextDependencyPropertyChangedHandlerId++;
    m_dependencyPropertyChangedHandlers[&property].emplace(handlerId, std::move(handler));
    return handlerId;
}

void DependencyObject::RemoveDependencyPropertyChangedHandler(const DependencyProperty& property,
                                                            std::size_t handlerId)
{
    if (auto it = m_dependencyPropertyChangedHandlers.find(&property); it != m_dependencyPropertyChangedHandlers.end()) {
        it->second.erase(handlerId);
        if (it->second.empty()) {
            m_dependencyPropertyChangedHandlers.erase(it);
        }
    }
}

const std::any& DependencyObject::ReadLocalValue(const DependencyProperty& property) const
{
    static const std::any empty;
    auto it = m_localValues.find(&property);
    if (it == m_localValues.end()) {
        return empty;
    }
    return it->second;
}

PropertyValueSource DependencyObject::GetValueSource(const DependencyProperty& property) const
{
    if (m_animatedValues.find(&property) != m_animatedValues.end()) {
        return PropertyValueSource::Animation;
    }
    if (m_localValues.find(&property) != m_localValues.end()) {
        return PropertyValueSource::Local;
    }
    if (m_expressionValues.find(&property) != m_expressionValues.end()) {
        return PropertyValueSource::Expression;
    }
    if (m_triggerValues.find(&property) != m_triggerValues.end()) {
        return PropertyValueSource::Trigger;
    }
    if (m_styleValues.find(&property) != m_styleValues.end()) {
        return PropertyValueSource::Style;
    }
    return PropertyValueSource::Default;
}

std::any DependencyObject::GetRawValue(const DependencyProperty& property) const
{
    if (auto it = m_animatedValues.find(&property); it != m_animatedValues.end()) {
        return it->second;
    }
    if (auto it = m_localValues.find(&property); it != m_localValues.end()) {
        return it->second;
    }
    if (auto it = m_expressionValues.find(&property); it != m_expressionValues.end()) {
        return it->second;
    }
    if (auto it = m_triggerValues.find(&property); it != m_triggerValues.end()) {
        return it->second;
    }
    if (auto it = m_styleValues.find(&property); it != m_styleValues.end()) {
        return it->second;
    }
    if (HasFlag(property.GetMetadata().Flags, PropertyFlags::Inherits)) {
        if (auto parent = GetInheritanceParent()) {
            return parent->GetRawValue(property);
        }
    }
    if (property.GetMetadata().DefaultValue.has_value()) {
        return property.GetMetadata().DefaultValue;
    }
    return std::any{};
}

void DependencyObject::SetValue(const DependencyProperty& property, const std::any& value)
{
    std::any convertedValue;
    if (Properties::TryConvertValue(property, value, convertedValue)) {
        SetValueInternal(property, std::move(convertedValue), PropertyValueSource::Local);
        return;
    }
    SetValueInternal(property, value, PropertyValueSource::Local);
}

void DependencyObject::SetValue(const DependencyProperty* property, const std::any& value)
{
    if (!property) {
        return;
    }
    SetValueInternal(*property, value, PropertyValueSource::Local);
}

const DependencyProperty* DependencyObject::FindDependencyProperty(const std::string& propertyName) const
{
    return DependencyPropertyRegistry::Get().FindProperty(propertyName, *this);
}

void DependencyObject::SetBinding(const std::string& propertyName, const Data::Binding& binding)
{
    if (const auto* property = FindDependencyProperty(propertyName)) {
        SetBinding(*property, binding);
    }
}

void DependencyObject::ClearBinding(const std::string& propertyName)
{
    if (const auto* property = FindDependencyProperty(propertyName)) {
        ClearBinding(*property);
    }
}

void DependencyObject::SetValueExpression(const DependencyProperty& property, const std::any& value)
{
    SetValueInternal(property, value, PropertyValueSource::Expression);
}

void DependencyObject::SetValueExpression(const DependencyProperty* property, const std::any& value)
{
    if (!property) {
        const std::wstring title = std::wstring(NeoWPF::Diagnostics::DeveloperWarnings::kInvalidDependencyPropertyCode) + L": invalid dependency property used for expression value";
        NeoWPF::Diagnostics::QueueDeveloperWarning(title,
            L"SetValueExpression was called with an invalid or unregistered DependencyProperty. Check that the target property exists and is registered before applying expression values.",
            NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
        return;
    }
    SetValueInternal(*property, value, PropertyValueSource::Expression);
}

void DependencyObject::SetValueAnimated(const DependencyProperty& property, const std::any& value)
{
    SetValueInternal(property, value, PropertyValueSource::Animation);
}

void DependencyObject::ClearAnimatedValue(const DependencyProperty& property)
{
    const auto animatedValue = m_animatedValues.find(&property);
    if (animatedValue == m_animatedValues.end()) {
        return;
    }

    const std::any oldValue = animatedValue->second;
    m_animatedValues.erase(animatedValue);
    const auto& metadata = property.GetMetadata();
    const std::any newValue = GetRawValue(property);
    const bool previousApplying = m_isApplyingPropertyValue;
    m_isApplyingPropertyValue = true;
    if (metadata.PropertyChangedCallback) {
        DependencyPropertyChangedEventArgs args{&property, oldValue, newValue, PropertyValueSource::Animation};
        metadata.PropertyChangedCallback(this, args);
    }
    if (auto handlers = m_dependencyPropertyChangedHandlers.find(&property); handlers != m_dependencyPropertyChangedHandlers.end()) {
        for (const auto& [id, handler] : handlers->second) {
            if (handler) {
                handler(oldValue, newValue);
            }
        }
    }
    m_isApplyingPropertyValue = previousApplying;
    OnDependencyPropertyChanged(property, metadata, oldValue, newValue, PropertyValueSource::Animation);
}

void DependencyObject::SetBinding(const DependencyProperty& property, const Data::Binding& binding)
{
    Data::BindingOperations::SetBinding(*this, property, binding);
}

void DependencyObject::SetBinding(const DependencyProperty* property, const Data::Binding& binding)
{
    if (!property) {
        const std::wstring title = std::wstring(NeoWPF::Diagnostics::DeveloperWarnings::kInvalidDependencyPropertyCode) + L": invalid dependency property used for binding";
        NeoWPF::Diagnostics::QueueDeveloperWarning(title,
            L"SetBinding was called with an invalid or unregistered DependencyProperty. Verify that the bound property exists and is registered for the target.",
            NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
        return;
    }
    Data::BindingOperations::SetBinding(*this, *property, binding);
}

void DependencyObject::ClearBinding(const DependencyProperty& property)
{
    Data::BindingOperations::ClearBinding(*this, property);
}

void DependencyObject::ClearBinding(const DependencyProperty* property)
{
    if (!property) {
        return;
    }
    Data::BindingOperations::ClearBinding(*this, *property);
}

void DependencyObject::ClearValue(const DependencyProperty& property)
{
    m_localValues.erase(&property);
    m_styleValues.erase(&property);
    m_expressionValues.erase(&property);
}

void DependencyObject::ClearValue(const DependencyProperty* property)
{
    if (!property) {
        return;
    }
    ClearValue(*property);
}

const std::any& DependencyObject::ReadLocalValue(const DependencyProperty* property) const
{
    static const std::any empty;
    if (!property) {
        return empty;
    }
    return ReadLocalValue(*property);
}

PropertyValueSource DependencyObject::GetValueSource(const DependencyProperty* property) const
{
    if (!property) {
        return PropertyValueSource::Default;
    }
    return GetValueSource(*property);
}

std::any DependencyObject::GetRawValue(const DependencyProperty* property) const
{
    if (!property) {
        return std::any{};
    }
    return GetRawValue(*property);
}


template bool DependencyObject::GetValue<bool>(const DependencyProperty&) const;
template int DependencyObject::GetValue<int>(const DependencyProperty&) const;
template float DependencyObject::GetValue<float>(const DependencyProperty&) const;
template std::wstring DependencyObject::GetValue<std::wstring>(const DependencyProperty&) const;
template std::any DependencyObject::GetValue<std::any>(const DependencyProperty&) const;
template LayoutSizeMode DependencyObject::GetValue<LayoutSizeMode>(const DependencyProperty&) const;
template CornerRadius DependencyObject::GetValue<CornerRadius>(const DependencyProperty&) const;
template Spacing DependencyObject::GetValue<Spacing>(const DependencyProperty&) const;
template Thickness DependencyObject::GetValue<Thickness>(const DependencyProperty&) const;
template HorizontalAlignment DependencyObject::GetValue<HorizontalAlignment>(const DependencyProperty&) const;
template VerticalAlignment DependencyObject::GetValue<VerticalAlignment>(const DependencyProperty&) const;

template void DependencyObject::SetValue<bool>(const DependencyProperty&, const bool&);
template void DependencyObject::SetValue<int>(const DependencyProperty&, const int&);
template void DependencyObject::SetValue<float>(const DependencyProperty&, const float&);
template void DependencyObject::SetValue<std::wstring>(const DependencyProperty&, const std::wstring&);
template void DependencyObject::SetValue<std::any>(const DependencyProperty&, const std::any&);
template void DependencyObject::SetValue<LayoutSizeMode>(const DependencyProperty&, const LayoutSizeMode&);
template void DependencyObject::SetValue<CornerRadius>(const DependencyProperty&, const CornerRadius&);
template void DependencyObject::SetValue<Spacing>(const DependencyProperty&, const Spacing&);
template void DependencyObject::SetValue<Thickness>(const DependencyProperty&, const Thickness&);
template void DependencyObject::SetValue<HorizontalAlignment>(const DependencyProperty&, const HorizontalAlignment&);
template void DependencyObject::SetValue<VerticalAlignment>(const DependencyProperty&, const VerticalAlignment&);
template NeoWPF::UI::Color DependencyObject::GetValue<NeoWPF::UI::Color>(const DependencyProperty&) const;
template void DependencyObject::SetValue<NeoWPF::UI::Color>(const DependencyProperty&, const NeoWPF::UI::Color&);

}
