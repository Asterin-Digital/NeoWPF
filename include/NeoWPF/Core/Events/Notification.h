/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <functional>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace NeoWPF::Core {

/*
    @summary
    Stores subscribers and publishes typed notifications.

    @details
    Subscription identifiers are local to each Notification instance. A cached immutable
    snapshot lets Publish invoke handlers without taking a lock or allocating.
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
        RebuildSnapshot();
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
        if (m_subscribers.erase(subscriptionId) != 0) {
            RebuildSnapshot();
        }
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
        const auto snapshot = std::atomic_load_explicit(&m_snapshot, std::memory_order_acquire);
        for (std::size_t index = 0; index < snapshot->inlineCount; ++index) {
            snapshot->inlineHandlers[index](args...);
        }
        for (const auto& handler : snapshot->overflowHandlers) {
            handler(args...);
        }
    }

private:
    struct SubscriberSnapshot {
        std::array<std::function<void(Args...)>, 4> inlineHandlers;
        std::size_t inlineCount = 0;
        std::vector<std::function<void(Args...)>> overflowHandlers;
    };

    void RebuildSnapshot()
    {
        auto snapshot = std::make_shared<SubscriberSnapshot>();
        const auto overflowCount = m_subscribers.size() > snapshot->inlineHandlers.size()
            ? m_subscribers.size() - snapshot->inlineHandlers.size()
            : 0;
        snapshot->overflowHandlers.reserve(overflowCount);

        for (const auto& [subscriptionId, handler] : m_subscribers) {
            (void)subscriptionId;
            if (snapshot->inlineCount < snapshot->inlineHandlers.size()) {
                snapshot->inlineHandlers[snapshot->inlineCount++] = handler;
            } else {
                snapshot->overflowHandlers.push_back(handler);
            }
        }

        std::shared_ptr<const SubscriberSnapshot> immutableSnapshot = std::move(snapshot);
        std::atomic_store_explicit(&m_snapshot, std::move(immutableSnapshot),
                                   std::memory_order_release);
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::size_t, std::function<void(Args...)>> m_subscribers;
    std::shared_ptr<const SubscriberSnapshot> m_snapshot{
        std::make_shared<const SubscriberSnapshot>()};
    std::size_t m_nextSubscriptionId = 1;
};

}
