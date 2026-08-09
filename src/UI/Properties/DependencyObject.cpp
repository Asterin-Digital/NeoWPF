#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Data/BindingExpression.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <stdexcept>

namespace NeoWPF::UI::Properties {

DependencyProperty::DependencyProperty(const std::string& name,
                                       const std::type_index& propertyType,
                                       const std::type_index& ownerType,
                                       const PropertyMetadata& metadata)
    : m_propertyType(propertyType)
    , m_ownerType(ownerType)
    , m_name(name)
    , m_metadata(metadata)
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
    for (const auto& property : m_properties) {
        if (property->GetName() != name) {
            continue;
        }
        if (property->GetOwnerType() == ownerType) {
            return property.get();
        }
    }

    for (const auto& property : m_properties) {
        if (property->GetName() != name) {
            continue;
        }

        const auto it = m_ownerTypeChecks.find(property->GetOwnerType());
        if (it != m_ownerTypeChecks.end() && it->second(&ownerInstance)) {
            return property.get();
        }
    }

    return nullptr;
}

void DependencyObject::SetValueInternal(const DependencyProperty& property,
                                       std::any value,
                                       PropertyValueSource source)
{
    std::any oldValue = GetRawValue(property);

    if (source == PropertyValueSource::Expression) {
        m_localValues.erase(&property);
    } else if (source != PropertyValueSource::Local) {
        ClearBindingExpression(property);
    }

    if (source == PropertyValueSource::Style) {
        m_styleValues[&property] = std::move(value);
    } else if (source == PropertyValueSource::Local) {
        m_localValues[&property] = std::move(value);
    } else if (source == PropertyValueSource::Expression) {
        m_expressionValues[&property] = std::move(value);
    } else {
        m_localValues[&property] = std::move(value);
    }

    const auto& metadata = property.GetMetadata();
    const std::any newValue = GetRawValue(property);

    const bool previousApplying = m_isApplyingPropertyValue;
    m_isApplyingPropertyValue = true;
    if (metadata.PropertyChangedCallback) {
        DependencyPropertyChangedEventArgs args{&property, std::move(oldValue), newValue, source};
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

    OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);
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
    if (m_localValues.find(&property) != m_localValues.end()) {
        return PropertyValueSource::Local;
    }
    if (m_expressionValues.find(&property) != m_expressionValues.end()) {
        return PropertyValueSource::Expression;
    }
    if (m_styleValues.find(&property) != m_styleValues.end()) {
        return PropertyValueSource::Style;
    }
    return PropertyValueSource::Default;
}

std::any DependencyObject::GetRawValue(const DependencyProperty& property) const
{
    if (auto it = m_localValues.find(&property); it != m_localValues.end()) {
        return it->second;
    }
    if (auto it = m_expressionValues.find(&property); it != m_expressionValues.end()) {
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
    SetValueInternal(property, value, PropertyValueSource::Local);
}

void DependencyObject::SetValue(const DependencyProperty* property, const std::any& value)
{
    if (!property) {
        return;
    }
    SetValueInternal(*property, value, PropertyValueSource::Local);
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
template NeoWPF::UI::Thickness DependencyObject::GetValue<NeoWPF::UI::Thickness>(const DependencyProperty&) const;
template void DependencyObject::SetValue<NeoWPF::UI::Thickness>(const DependencyProperty&, const NeoWPF::UI::Thickness&);

}
