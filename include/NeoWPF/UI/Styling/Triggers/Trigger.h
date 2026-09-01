#pragma once

#include <any>
#include <mutex>
#include <string>

#include <NeoWPF/UI/Styling/Triggers/TriggerBase.h>

namespace NeoWPF::UI::Styling::Triggers {

class Trigger : public TriggerBase {
public:
    Trigger() = default;
    Trigger(const std::string& propertyName, const std::any& value);

    void SetProperty(const std::string& propertyName) noexcept;
    std::string GetProperty() const;

    void SetValue(const std::any& value);
    std::any GetValue() const;

    bool Evaluate(UIElement* element) const override;

private:
    mutable std::mutex m_triggerMutex;
    std::string m_propertyName;
    std::any m_value;
};

}
