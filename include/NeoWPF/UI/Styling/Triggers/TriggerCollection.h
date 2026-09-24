/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <mutex>
#include <vector>

namespace NeoWPF::UI::Styling::Triggers {

class TriggerBase;

/*
    @summary
    Stores a collection of style triggers.
*/
class TriggerCollection {
public:
    /*
        @summary
        Adds a trigger to the collection.

        @param trigger
        Trigger to add.
    */
    void Add(const std::shared_ptr<TriggerBase>& trigger);

    /*
        @summary
        Removes a trigger from the collection.

        @param trigger
        Trigger to remove.
    */
    void Remove(const std::shared_ptr<TriggerBase>& trigger);

    /*
        @summary
        Removes all triggers from the collection.
    */
    void Clear();

    std::vector<std::shared_ptr<TriggerBase>> GetTriggers() const;

private:
    mutable std::mutex m_triggersMutex;
    std::vector<std::shared_ptr<TriggerBase>> m_triggers;
};

}
