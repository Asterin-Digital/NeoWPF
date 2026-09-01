#include <NeoWPF/UI/Styling/Triggers/TriggerCollection.h>
#include <NeoWPF/UI/Styling/Triggers/TriggerBase.h>

#include <algorithm>

namespace NeoWPF::UI::Styling::Triggers {

void TriggerCollection::Add(const std::shared_ptr<TriggerBase>& trigger) {
    if (!trigger) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_triggersMutex);

    auto existing = std::find_if(m_triggers.begin(), m_triggers.end(), [&](const auto& candidate) {
        return candidate.get() == trigger.get();
    });

    if (existing == m_triggers.end()) {
        m_triggers.push_back(trigger);
    }
}

void TriggerCollection::Remove(const std::shared_ptr<TriggerBase>& trigger) {
    if (!trigger) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_triggersMutex);

    m_triggers.erase(std::remove_if(m_triggers.begin(), m_triggers.end(), [&](const auto& candidate) {
        return candidate.get() == trigger.get();
    }), m_triggers.end());
}

void TriggerCollection::Clear() {
    std::lock_guard<std::mutex> lock(m_triggersMutex);
    m_triggers.clear();
}

std::vector<std::shared_ptr<TriggerBase>> TriggerCollection::GetTriggers() const {
    std::lock_guard<std::mutex> lock(m_triggersMutex);
    return m_triggers;
}

}
