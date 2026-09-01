#pragma once

#include <any>
#include <mutex>
#include <string>

#include <NeoWPF/UI/Styling/Triggers/TriggerBase.h>

namespace NeoWPF::UI::Styling::Triggers {

class DataTrigger : public TriggerBase {
public:
    DataTrigger() = default;
    DataTrigger(const std::string& bindingPath, const std::any& value);

    void SetBindingPath(const std::string& bindingPath) noexcept;
    std::string GetBindingPath() const;

    void SetValue(const std::any& value);
    std::any GetValue() const;

    bool Evaluate(UIElement* element) const override;

private:
    mutable std::mutex m_dataMutex;
    std::string m_bindingPath;
    std::any m_value;
};

}
