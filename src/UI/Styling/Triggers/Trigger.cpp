#include <NeoWPF/UI/Styling/Triggers/Trigger.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include "AnyEquals.h"

namespace NeoWPF::UI::Styling::Triggers {

Trigger::Trigger(const std::string& propertyName, const std::any& value)
    : m_propertyName(propertyName)
    , m_value(value)
{
}

void Trigger::SetProperty(const std::string& propertyName) noexcept {
    std::lock_guard<std::mutex> lock(m_triggerMutex);
    m_propertyName = propertyName;
}

std::string Trigger::GetProperty() const {
    std::lock_guard<std::mutex> lock(m_triggerMutex);
    return m_propertyName;
}

void Trigger::SetValue(const std::any& value) {
    std::lock_guard<std::mutex> lock(m_triggerMutex);
    m_value = value;
}

std::any Trigger::GetValue() const {
    std::lock_guard<std::mutex> lock(m_triggerMutex);
    return m_value;
}

bool Trigger::Evaluate(UIElement* element) const {
    if (!element) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_triggerMutex);

    if (m_propertyName.empty()) {
        return false;
    }

    const auto* property = element->FindDependencyProperty(m_propertyName);
    if (!property) {
        return false;
    }

    const auto currentValue = element->GetRawValue(*property);
    return Detail::AnyEquals(currentValue, m_value);
}

}
