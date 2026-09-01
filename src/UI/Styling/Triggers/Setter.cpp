#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Styling::Triggers {

Setter::Setter(const std::string& propertyName, const std::any& value)
    : m_propertyName(propertyName)
    , m_value(value)
{
}

void Setter::SetProperty(const std::string& propertyName) noexcept {
    m_propertyName = propertyName;
}

const std::string& Setter::GetProperty() const noexcept {
    return m_propertyName;
}

void Setter::SetValue(const std::any& value) {
    m_value = value;
}

const std::any& Setter::GetValue() const noexcept {
    return m_value;
}

void Setter::Apply(UIElement* element) const {
    if (!element || m_propertyName.empty()) {
        return;
    }

    const auto* property = element->FindDependencyProperty(m_propertyName);
    if (!property) {
        return;
    }

    element->SetValue(*property, m_value);
}

}
