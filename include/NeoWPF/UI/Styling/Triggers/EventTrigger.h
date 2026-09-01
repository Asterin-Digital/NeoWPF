#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <NeoWPF/UI/Styling/Triggers/TriggerBase.h>

namespace NeoWPF::UI::Styling::Triggers {

class TriggerAction;

class EventTrigger : public TriggerBase {
public:
    EventTrigger() = default;
    explicit EventTrigger(const std::string& routedEventName);

    void SetRoutedEventName(const std::string& routedEventName) noexcept;
    std::string GetRoutedEventName() const;

    void AddAction(const std::shared_ptr<TriggerAction>& action);
    std::vector<std::shared_ptr<TriggerAction>> GetActions() const;

    bool Evaluate(UIElement* element) const override;
    void Invoke(UIElement* element) const;

private:
    mutable std::mutex m_eventMutex;
    std::string m_routedEventName;
    std::vector<std::shared_ptr<TriggerAction>> m_actions;
};

}
