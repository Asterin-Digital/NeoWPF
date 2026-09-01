#include <NeoWPF/UI/Styling/Triggers/Trigger.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

#include <string>

namespace NeoWPF::UI::Styling::Triggers {
namespace {

bool AnyEquals(const std::any& left, const std::any& right) {
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

    return false;
}

}

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
    return AnyEquals(currentValue, m_value);
}

}
