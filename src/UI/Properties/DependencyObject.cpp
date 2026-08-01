#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/Color.h>
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
    const DependencyProperty* fallback = nullptr;
    for (const auto& property : m_properties) {
        if (property->GetName() != name) {
            continue;
        }
        if (property->GetOwnerType() == ownerType) {
            return property.get();
        }
        if (!fallback) {
            fallback = property.get();
        }
    }
    return fallback;
}

void DependencyObject::SetValueInternal(const DependencyProperty& property,
                                       std::any value,
                                       PropertyValueSource source)
{
    std::any oldValue = GetRawValue(property);

    if (source == PropertyValueSource::Style) {
        m_styleValues[&property] = std::move(value);
    } else if (source == PropertyValueSource::Local) {
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
    m_isApplyingPropertyValue = previousApplying;

    OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);
}

void DependencyObject::ClearValue(const DependencyProperty& property)
{
    m_localValues.erase(&property);
    m_styleValues.erase(&property);
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
    if (auto it = m_styleValues.find(&property); it != m_styleValues.end()) {
        return it->second;
    }
    if (property.GetMetadata().DefaultValue.has_value()) {
        return property.GetMetadata().DefaultValue;
    }
    return std::any{};
}

template<typename T>
T DependencyObject::GetValue(const DependencyProperty& property) const
{
    if (property.GetPropertyType() != typeid(T)) {
        throw std::bad_any_cast();
    }

    if (auto it = m_localValues.find(&property); it != m_localValues.end()) {
        return std::any_cast<T>(it->second);
    }

    if (auto it = m_styleValues.find(&property); it != m_styleValues.end()) {
        return std::any_cast<T>(it->second);
    }

    if (property.GetMetadata().DefaultValue.has_value()) {
        return std::any_cast<T>(property.GetMetadata().DefaultValue);
    }

    return T{};
}

template<typename T>
void DependencyObject::SetValue(const DependencyProperty& property, const T& value)
{
    if (property.GetPropertyType() != typeid(T)) {
        throw std::bad_any_cast();
    }
    SetValueInternal(property, std::any(value), PropertyValueSource::Local);
}

void DependencyObject::SetValue(const DependencyProperty& property, const std::any& value)
{
    SetValueInternal(property, value, PropertyValueSource::Local);
}


template bool DependencyObject::GetValue<bool>(const DependencyProperty&) const;
template int DependencyObject::GetValue<int>(const DependencyProperty&) const;
template float DependencyObject::GetValue<float>(const DependencyProperty&) const;
template std::wstring DependencyObject::GetValue<std::wstring>(const DependencyProperty&) const;
template std::any DependencyObject::GetValue<std::any>(const DependencyProperty&) const;
template LayoutSizeMode DependencyObject::GetValue<LayoutSizeMode>(const DependencyProperty&) const;
template CornerRadius DependencyObject::GetValue<CornerRadius>(const DependencyProperty&) const;
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
template void DependencyObject::SetValue<Thickness>(const DependencyProperty&, const Thickness&);
template void DependencyObject::SetValue<HorizontalAlignment>(const DependencyProperty&, const HorizontalAlignment&);
template void DependencyObject::SetValue<VerticalAlignment>(const DependencyProperty&, const VerticalAlignment&);
template NeoWPF::UI::Color DependencyObject::GetValue<NeoWPF::UI::Color>(const DependencyProperty&) const;
template void DependencyObject::SetValue<NeoWPF::UI::Color>(const DependencyProperty&, const NeoWPF::UI::Color&);
template NeoWPF::UI::Thickness DependencyObject::GetValue<NeoWPF::UI::Thickness>(const DependencyProperty&) const;
template void DependencyObject::SetValue<NeoWPF::UI::Thickness>(const DependencyProperty&, const NeoWPF::UI::Thickness&);

}
