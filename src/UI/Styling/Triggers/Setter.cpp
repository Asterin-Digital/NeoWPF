#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Styling::Triggers {

Setter::Setter(const std::string& propertyName, const std::any& value)
{
    SetProperty(propertyName);
    SetValue(value);
}

Setter::Setter(const Setter& other)
{
    SetProperty(other.GetProperty());
    SetValue(other.GetValue());
}

Setter& Setter::operator=(const Setter& other)
{
    if (this != &other) {
        SetProperty(other.GetProperty());
        SetValue(other.GetValue());
    }
    return *this;
}

Setter::Setter(Setter&& other) noexcept
{
    SetProperty(other.GetProperty());
    SetValue(other.GetValue());
}

Setter& Setter::operator=(Setter&& other) noexcept
{
    if (this != &other) {
        SetProperty(other.GetProperty());
        SetValue(other.GetValue());
    }
    return *this;
}

void Setter::SetProperty(const std::string& propertyName) noexcept {
    const std::string current = GetProperty();
    if (current == propertyName) {
        return;
    }
    DependencyObject::SetValue(PropertyProperty(), propertyName);
}

std::string Setter::GetProperty() const {
    static const std::string empty;
    const std::any& value = GetRawValue(PropertyProperty());
    if (value.type() == typeid(std::string)) {
        return std::any_cast<std::string>(value);
    }
    if (value.type() == typeid(const char*)) {
        return empty;
    }
    return empty;
}

void Setter::SetValue(const std::any& value) {
    std::any normalized = value;
    if (value.type() == typeid(const char*)) {
        normalized = std::string(std::any_cast<const char*>(value));
    }
    if (normalized.has_value() && GetRawValue(ValueProperty()).type() == normalized.type() &&
        Properties::ValuesEqual(GetRawValue(ValueProperty()), normalized)) {
        return;
    }
    DependencyObject::SetValue(ValueProperty(), normalized);
}

std::any Setter::GetValue() const {
    return GetRawValue(ValueProperty());
}

std::size_t Setter::AddChangedHandler(std::function<void()> handler)
{
    return m_changed->Subscribe(std::move(handler));
}

void Setter::RemoveChangedHandler(std::size_t handlerId)
{
    m_changed->Unsubscribe(handlerId);
}

void Setter::OnDependencyPropertyChanged(
    const Properties::DependencyProperty& property,
    const Properties::PropertyMetadata& metadata,
    const std::any& oldValue,
    const std::any& newValue,
    Properties::PropertyValueSource source)
{
    Properties::DependencyObject::OnDependencyPropertyChanged(
        property, metadata, oldValue, newValue, source);
    m_changed->Publish();
}

void Setter::Apply(UIElement* element) const {
    const std::string propertyName = GetProperty();
    if (!element || propertyName.empty()) {
        return;
    }

    const auto* property = element->FindDependencyProperty(propertyName);
    if (!property) {
        return;
    }

    element->SetTriggerPropertyValue(propertyName, GetValue());
}

}
