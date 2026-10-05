#include <gtest/gtest.h>

#include <NeoWPF/Core/Events/Notification.h>

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

namespace {

using NeoWPF::Core::Notification;

TEST(NotificationTests, RejectsEmptyHandlers)
{
    Notification<> notification;

    EXPECT_EQ(notification.Subscribe(std::function<void()>{}), 0u);
}

TEST(NotificationTests, PublishesTypedArguments)
{
    Notification<int, int> notification;
    int result = 0;
    notification.Subscribe([&result](int left, int right) {
        result = left + right;
    });

    notification.Publish(20, 22);

    EXPECT_EQ(result, 42);
}

TEST(NotificationTests, UnsubscribePreventsFutureCallbacks)
{
    Notification<> notification;
    int callbackCount = 0;
    const auto subscriptionId = notification.Subscribe([&callbackCount] {
        ++callbackCount;
    });

    ASSERT_NE(subscriptionId, 0u);
    notification.Publish();
    notification.Unsubscribe(subscriptionId);
    notification.Publish();

    EXPECT_EQ(callbackCount, 1);
}

TEST(NotificationTests, SupportsReentrantSubscriptionChanges)
{
    Notification<> notification;
    std::size_t firstSubscription = 0;
    int secondCallbackCount = 0;
    firstSubscription = notification.Subscribe([&] {
        notification.Unsubscribe(firstSubscription);
        notification.Subscribe([&secondCallbackCount] {
            ++secondCallbackCount;
        });
    });

    notification.Publish();
    EXPECT_EQ(secondCallbackCount, 0);

    notification.Publish();
    EXPECT_EQ(secondCallbackCount, 1);
}

TEST(NotificationTests, SupportsConcurrentSubscribePublishAndUnsubscribe)
{
    Notification<int> notification;
    std::atomic<int> callbackCount{0};
    std::vector<std::thread> workers;

    for (int workerIndex = 0; workerIndex < 4; ++workerIndex) {
        workers.emplace_back([&] {
            for (int iteration = 0; iteration < 250; ++iteration) {
                const auto subscriptionId = notification.Subscribe([&callbackCount](int value) {
                    callbackCount.fetch_add(value, std::memory_order_relaxed);
                });
                notification.Publish(1);
                notification.Unsubscribe(subscriptionId);
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    EXPECT_GT(callbackCount.load(std::memory_order_relaxed), 0);
}

}
