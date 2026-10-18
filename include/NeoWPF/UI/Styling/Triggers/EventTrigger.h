/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <NeoWPF/UI/Styling/Triggers/TriggerBase.h>

namespace NeoWPF::UI::Styling::Triggers {

class TriggerAction;

/*
    @summary
    Invokes actions when a routed event occurs.
*/
class EventTrigger : public TriggerBase {
public:
    EventTrigger() = default;

    /*
        @summary
        Creates an event trigger.

        @param routedEventName
        Routed event name.
    */
    explicit EventTrigger(const std::string& routedEventName);

    /*
        @summary
        Sets the routed event observed by the trigger.

        @param routedEventName
        Routed event name.
    */
    void SetRoutedEventName(const std::string& routedEventName) noexcept;
    std::string GetRoutedEventName() const;

    /*
        @summary
        Adds an action invoked by the trigger.

        @param action
        Action to add.
    */
    void AddAction(const std::shared_ptr<TriggerAction>& action);
    std::vector<std::shared_ptr<TriggerAction>> GetActions() const;

    bool Evaluate(UIElement* element) const override;
    /*
        @summary
        Invokes the trigger actions for an element.

        @param element
        Element that raised the event.
    */
    void Invoke(UIElement* element) const;

private:
    mutable std::mutex m_eventMutex;
    std::string m_routedEventName;
    std::vector<std::shared_ptr<TriggerAction>> m_actions;
};

}
