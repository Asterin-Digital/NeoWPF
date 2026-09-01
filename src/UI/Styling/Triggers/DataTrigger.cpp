#include <NeoWPF/UI/Styling/Triggers/DataTrigger.h>
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

DataTrigger::DataTrigger(const std::string& bindingPath, const std::any& value)
    : m_bindingPath(bindingPath)
    , m_value(value)
{
}

void DataTrigger::SetBindingPath(const std::string& bindingPath) noexcept {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    m_bindingPath = bindingPath;
}

std::string DataTrigger::GetBindingPath() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_bindingPath;
}

void DataTrigger::SetValue(const std::any& value) {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    m_value = value;
}

std::any DataTrigger::GetValue() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_value;
}

bool DataTrigger::Evaluate(UIElement* element) const {
    if (!element) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_dataMutex);

    if (m_bindingPath.empty()) {
        return false;
    }

    const auto* property = element->FindDependencyProperty(m_bindingPath);
    if (!property) {
        return false;
    }

    return AnyEquals(element->GetRawValue(*property), m_value);
}

}
