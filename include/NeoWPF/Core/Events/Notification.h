/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace NeoWPF::Core {

/*
    @summary
    Stores subscribers and publishes typed notifications.

    @details
    Subscription identifiers are local to each Notification instance. Handlers are copied under a shared lock and invoked after the lock is released.
*/
template <typename... Args>
class Notification {
public:
    /*
        @summary
        Subscribes a handler to notifications.

        @param handler
        Callback invoked by Publish.

        @returns
        A nonzero subscription identifier, or zero when the handler is empty.
    */
    std::size_t Subscribe(std::function<void(Args...)> handler)
    {
        if (!handler) {
            return 0;
        }

        std::unique_lock lock(m_mutex);
        const auto subscriptionId = m_nextSubscriptionId++;
        m_subscribers.emplace(subscriptionId, std::move(handler));
        return subscriptionId;
    }

    /*
        @summary
        Removes a handler from future notification snapshots.

        @param subscriptionId
        Identifier returned by Subscribe.
    */
    void Unsubscribe(std::size_t subscriptionId)
    {
        if (subscriptionId == 0) {
            return;
        }

        std::unique_lock lock(m_mutex);
        m_subscribers.erase(subscriptionId);
    }

    /*
        @summary
        Publishes a notification to a snapshot of the current subscribers.

        @param args
        Values supplied to each subscriber.

        @details
        Handlers run synchronously on the publishing thread. Concurrent Publish calls may invoke a handler concurrently. Unsubscribe does not cancel a handler already copied into a publish snapshot.
    */
    void Publish(Args... args) const
    {
        std::vector<std::function<void(Args...)>> subscribers;
        {
            std::shared_lock lock(m_mutex);
            subscribers.reserve(m_subscribers.size());
            for (const auto& [subscriptionId, handler] : m_subscribers) {
                (void)subscriptionId;
                if (handler) {
                    subscribers.push_back(handler);
                }
            }
        }

        for (const auto& handler : subscribers) {
            handler(args...);
        }
    }

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::size_t, std::function<void(Args...)>> m_subscribers;
    std::size_t m_nextSubscriptionId = 1;
};

}
