#include <gtest/gtest.h>

#include <NeoWPF/Core/Threading/BackgroundExecutor.h>
#include <NeoWPF/Core/Threading/Cancellation.h>
#include <NeoWPF/Core/Threading/Task.h>
#include <NeoWPF/Core/Threading/ThreadAffinity.h>
#include <NeoWPF/Core/Threading/MainThreadExecutor.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

TEST(ThreadingTests, RegistersMainThreadAffinity)
{
    NeoWPF::Core::Threading::ThreadAffinity::RegisterMainThread();

    EXPECT_TRUE(NeoWPF::Core::Threading::ThreadAffinity::IsMainThread());
    EXPECT_FALSE(NeoWPF::Core::Threading::ThreadAffinity::IsRenderThread());

    NeoWPF::Core::Threading::ThreadAffinity::ClearMainThread();
}

TEST(ThreadingTests, DetectsRenderThreadRegistration)
{
    NeoWPF::Core::Threading::ThreadAffinity::RegisterRenderThread();

    EXPECT_TRUE(NeoWPF::Core::Threading::ThreadAffinity::IsRenderThread());
    EXPECT_FALSE(NeoWPF::Core::Threading::ThreadAffinity::IsMainThread());

    NeoWPF::Core::Threading::ThreadAffinity::ClearRenderThread();
}

TEST(ThreadingTests, RenderAffinityBelongsToTheRegisteredWorker)
{
    std::atomic<bool> workerIsRenderThread{false};
    std::thread renderWorker([&workerIsRenderThread] {
        NeoWPF::Core::Threading::ThreadAffinity::RegisterRenderThread();
        workerIsRenderThread =
            NeoWPF::Core::Threading::ThreadAffinity::IsRenderThread();
        NeoWPF::Core::Threading::ThreadAffinity::ClearRenderThread();
    });
    renderWorker.join();

    EXPECT_TRUE(workerIsRenderThread.load());
    EXPECT_FALSE(NeoWPF::Core::Threading::ThreadAffinity::IsRenderThread());
}

TEST(ThreadingTests, ThreadKindMatchesCurrentThread)
{
    const auto current = NeoWPF::Core::Threading::ThreadAffinity::GetCurrentThreadKind();
    EXPECT_EQ(current, NeoWPF::Core::Threading::ThreadKind::Unknown);

    NeoWPF::Core::Threading::ThreadAffinity::RegisterMainThread();
    EXPECT_EQ(NeoWPF::Core::Threading::ThreadAffinity::GetCurrentThreadKind(),
              NeoWPF::Core::Threading::ThreadKind::Main);

    NeoWPF::Core::Threading::ThreadAffinity::ClearMainThread();
}

TEST(ThreadingTests, MainExecutorDrainsPostedTasksInOrder)
{
    NeoWPF::Core::Threading::MainThreadExecutor executor;
    std::vector<int> order;

    ASSERT_TRUE(executor.Post([&order] { order.push_back(1); }));
    ASSERT_TRUE(executor.Post([&order] { order.push_back(2); }));

    EXPECT_EQ(executor.Drain(std::chrono::milliseconds(100)), 2u);
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
}

TEST(ThreadingTests, MainExecutorOnlyDrainsOnOwnerThread)
{
    NeoWPF::Core::Threading::MainThreadExecutor executor;
    std::atomic<bool> ran{false};

    ASSERT_TRUE(executor.Post([&ran] { ran = true; }));
    std::thread worker([&executor] {
        EXPECT_EQ(executor.Drain(std::chrono::milliseconds(100)), 0u);
    });
    worker.join();

    EXPECT_FALSE(ran.load());
    EXPECT_EQ(executor.Drain(std::chrono::milliseconds(100)), 1u);
    EXPECT_TRUE(ran.load());
}

TEST(ThreadingTests, MainExecutorRejectsTasksAfterStop)
{
    NeoWPF::Core::Threading::MainThreadExecutor executor;
    bool ran = false;

    ASSERT_TRUE(executor.Post([&ran] { ran = true; }));
    executor.Stop();

    EXPECT_TRUE(executor.IsStopped());
    EXPECT_FALSE(executor.Post([&ran] { ran = true; }));
    EXPECT_EQ(executor.Drain(std::chrono::milliseconds(100)), 0u);
    EXPECT_FALSE(ran);
}

TEST(ThreadingTests, BackgroundExecutorRunsTasksInQueueOrder)
{
    NeoWPF::Core::Threading::BackgroundExecutor executor(1);
    std::vector<int> order;
    std::mutex orderMutex;

    ASSERT_TRUE(executor.Post([&] {
        std::lock_guard<std::mutex> lock(orderMutex);
        order.push_back(1);
    }));
    ASSERT_TRUE(executor.Post([&] {
        std::lock_guard<std::mutex> lock(orderMutex);
        order.push_back(2);
    }));

    executor.Stop(NeoWPF::Core::Threading::ShutdownMode::DrainPending);
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
}

TEST(ThreadingTests, BackgroundExecutorSubmitReturnsResultAndException)
{
    NeoWPF::Core::Threading::BackgroundExecutor executor(1);
    auto result = executor.Submit([] { return 42; });
    EXPECT_EQ(result.get(), 42);

    auto failure = executor.Submit([]() -> int {
        throw std::runtime_error("failure");
    });
    EXPECT_THROW(failure.get(), std::runtime_error);
}

TEST(ThreadingTests, BackgroundExecutorRejectsWorkAfterStop)
{
    NeoWPF::Core::Threading::BackgroundExecutor executor(1);
    executor.Stop();

    EXPECT_TRUE(executor.IsStopped());
    EXPECT_FALSE(executor.Post([] {}));

    auto result = executor.Submit([] { return 42; });
    EXPECT_THROW(result.get(), std::runtime_error);
}

TEST(ThreadingTests, CancellationSourceCanRequestCancellation)
{
    NeoWPF::Core::Threading::CancellationSource source;
    const auto token = source.GetToken();

    EXPECT_FALSE(token.IsCancellationRequested());
    EXPECT_TRUE(source.RequestCancellation());
    EXPECT_FALSE(source.RequestCancellation());
    EXPECT_TRUE(token.IsCancellationRequested());
}

TEST(ThreadingTests, BackgroundExecutorPassesCancellationTokenToWork)
{
    NeoWPF::Core::Threading::BackgroundExecutor executor(1);
    NeoWPF::Core::Threading::CancellationSource source;
    auto token = source.GetToken();

    auto result = executor.Submit(token, [](NeoWPF::Core::Threading::CancellationToken taskToken) {
        return taskToken.IsCancellationRequested();
    });

    EXPECT_FALSE(result.get());
    source.RequestCancellation();
    executor.Stop();
}

TEST(ThreadingTests, CancelledPostDoesNotRun)
{
    NeoWPF::Core::Threading::BackgroundExecutor executor(1);
    NeoWPF::Core::Threading::CancellationSource source;
    std::atomic<bool> ran{false};
    source.RequestCancellation();

    ASSERT_TRUE(executor.Post(source.GetToken(), [&ran](auto) { ran = true; }));
    executor.Stop(NeoWPF::Core::Threading::ShutdownMode::DrainPending);

    EXPECT_FALSE(ran.load());
}

TEST(ThreadingTests, TaskReturnsResultAndReportsCompletion)
{
    NeoWPF::Core::Threading::BackgroundExecutor background(1);
    auto task = background.SubmitTask([] { return 42; });

    EXPECT_EQ(task.Get(), 42);
    EXPECT_TRUE(task.IsCompleted());
}

TEST(ThreadingTests, TaskContinuationRunsOnMainExecutor)
{
    NeoWPF::Core::Threading::BackgroundExecutor background(1);
    NeoWPF::Core::Threading::MainThreadExecutor main;
    std::atomic<int> value{0};

    auto task = background.SubmitTask([] { return 42; });
    ASSERT_TRUE(task.Then(main, [&value](int result) { value = result; }));

    EXPECT_EQ(task.Get(), 42);
    EXPECT_EQ(value.load(), 0);
    EXPECT_EQ(main.Drain(std::chrono::milliseconds(100)), 1u);
    EXPECT_EQ(value.load(), 42);
}

TEST(ThreadingTests, RejectedTaskCompletesWithAnException)
{
    NeoWPF::Core::Threading::BackgroundExecutor background(1);
    background.Stop();

    auto task = background.SubmitTask([] { return 42; });

    EXPECT_TRUE(task.IsCompleted());
    EXPECT_THROW(task.Get(), std::runtime_error);
}
