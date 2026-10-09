#include <NeoWPF/UI/Styling/Triggers/DataTrigger.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Styling::Triggers {

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

    return NeoWPF::UI::Properties::ValuesEqual(*element, *property, m_value);
}

}
