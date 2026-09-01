#include <NeoWPF/UI/Styling/Triggers/EventTrigger.h>
#include <NeoWPF/UI/Styling/Triggers/Actions/TriggerAction.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Styling::Triggers {

EventTrigger::EventTrigger(const std::string& routedEventName)
    : m_routedEventName(routedEventName)
{
}

void EventTrigger::SetRoutedEventName(const std::string& routedEventName) noexcept {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_routedEventName = routedEventName;
}

std::string EventTrigger::GetRoutedEventName() const {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    return m_routedEventName;
}

void EventTrigger::AddAction(const std::shared_ptr<TriggerAction>& action) {
    if (!action) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_actions.push_back(action);
}

std::vector<std::shared_ptr<TriggerAction>> EventTrigger::GetActions() const {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    return m_actions;
}

bool EventTrigger::Evaluate(UIElement* element) const {
    if (!element) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_eventMutex);
    return !m_routedEventName.empty();
}

void EventTrigger::Invoke(UIElement* element) const {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    for (const auto& action : m_actions) {
        if (action) {
            action->Invoke(element);
        }
    }
}

}
