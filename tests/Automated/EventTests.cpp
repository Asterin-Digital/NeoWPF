#include <gtest/gtest.h>

#include <NeoWPF/Core/Events/Event.h>

#include <string>
#include <vector>

namespace {

TEST(EventTests, InvokesSubscribedHandlersWithArguments)
{
    NeoWPF::Core::Event<int, std::string> event;
    int receivedNumber = 0;
    std::string receivedText;

    event += [&](int number, std::string text) {
        receivedNumber = number;
        receivedText = std::move(text);
    };

    event.Invoke(42, "payload");

    EXPECT_EQ(receivedNumber, 42);
    EXPECT_EQ(receivedText, "payload");
}

TEST(EventTests, InvokesHandlersInSubscriptionOrder)
{
    NeoWPF::Core::Event<int> event;
    std::vector<int> calls;

    event += [&](int value) { calls.push_back(value + 1); };
    event += [&](int value) { calls.push_back(value + 2); };
    event += [&](int value) { calls.push_back(value + 3); };

    event.Invoke(10);

    ASSERT_EQ(calls.size(), 3u);
    EXPECT_EQ(calls[0], 11);
    EXPECT_EQ(calls[1], 12);
    EXPECT_EQ(calls[2], 13);
}

TEST(EventTests, IgnoresEmptyHandlers)
{
    NeoWPF::Core::Event<> event;
    int invocationCount = 0;

    event += NeoWPF::Core::Event<>::Delegate{};
    event += [&]() { ++invocationCount; };
    event.Invoke();

    EXPECT_EQ(invocationCount, 1);
}

TEST(EventTests, AllowsDuplicateSubscriptions)
{
    NeoWPF::Core::Event<int> event;
    int invocationCount = 0;
    NeoWPF::Core::Event<int>::Delegate handler = [&](int) { ++invocationCount; };

    event += handler;
    event += handler;
    event.Invoke(0);

    EXPECT_EQ(invocationCount, 2);
}

TEST(EventTests, RemovesSubscribedHandler)
{
    NeoWPF::Core::Event<int> event;
    int invocationCount = 0;
    NeoWPF::Core::Event<int>::Delegate handler = [&](int) { ++invocationCount; };

    event += handler;
    event -= handler;
    event.Invoke(0);

    EXPECT_EQ(invocationCount, 0);
}

}