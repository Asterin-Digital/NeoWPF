#pragma once

#include <memory>
#include <mutex>
#include <vector>

namespace NeoWPF::UI::Styling::Triggers {

class TriggerBase;

class TriggerCollection {
public:
    void Add(const std::shared_ptr<TriggerBase>& trigger);
    void Remove(const std::shared_ptr<TriggerBase>& trigger);
    void Clear();

    std::vector<std::shared_ptr<TriggerBase>> GetTriggers() const;

private:
    mutable std::mutex m_triggersMutex;
    std::vector<std::shared_ptr<TriggerBase>> m_triggers;
};

}
