#include <gtest/gtest.h>

#include <NeoWPF/Platform/Android/AActivityInputQueue.h>
#include <NeoWPF/Platform/Android/AActivityLifecycle.h>

#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#if defined(__ANDROID__)
#include <NeoWPF/Platform/Android/AActivityHost.h>
#include <android/native_activity.h>

namespace NeoWPF::Platform::Android {
void ARegisterActivityCallbacks(ANativeActivityCallbacks& callbacks) noexcept;
}

int NeoWPFAndroidApplicationMain()
{
    return 0;
}
#endif

namespace {

void CountFinish(void* context) noexcept
{
    ++*static_cast<int*>(context);
}

struct SurfaceRecord {
    void* nativeWindow = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    int availableCount = 0;
    int destroyedCount = 0;
};

void RecordSurfaceAvailable(void* context, void* nativeWindow,
                            unsigned int width, unsigned int height) noexcept
{
    auto& record = *static_cast<SurfaceRecord*>(context);
    record.nativeWindow = nativeWindow;
    record.width = width;
    record.height = height;
    ++record.availableCount;
}

void RecordSurfaceDestroyed(void* context) noexcept
{
    ++static_cast<SurfaceRecord*>(context)->destroyedCount;
}

void RecordActivityDestroyed(void* context) noexcept
{
    ++*static_cast<int*>(context);
}

struct NativeWindowRecord {
    int referenceCount = 0;
    int width = 0;
    int height = 0;
};

void AcquireWindow(void* context) noexcept
{
    ++static_cast<NativeWindowRecord*>(context)->referenceCount;
}

void ReleaseWindow(void* context) noexcept
{
    --static_cast<NativeWindowRecord*>(context)->referenceCount;
}

int GetWindowWidth(void* context) noexcept
{
    return static_cast<NativeWindowRecord*>(context)->width;
}

int GetWindowHeight(void* context) noexcept
{
    return static_cast<NativeWindowRecord*>(context)->height;
}

struct BlockingSurfaceRecord {
    std::shared_ptr<std::promise<void>> callbackStarted;
    std::shared_future<void> continueCallback;
};

void BlockSurfaceAvailable(void* context, void*, unsigned int, unsigned int) noexcept
{
    auto& record = *static_cast<BlockingSurfaceRecord*>(context);
    record.callbackStarted->set_value();
    record.continueCallback.wait();
}

struct FakeActivityCallbacks {
    using Callback = void (*)();
    using InputQueueCallback = void (*)(void*, void*) noexcept;
    Callback onNativeWindowCreated = nullptr;
    Callback onNativeWindowResized = nullptr;
    Callback onNativeWindowDestroyed = nullptr;
    InputQueueCallback onInputQueueCreated = nullptr;
    InputQueueCallback onInputQueueDestroyed = nullptr;
    Callback onDestroy = nullptr;
};

void CreatedCallback() {}
void ResizedCallback() {}
void SurfaceDestroyedCallback() {}
void InputQueueCreatedCallback(void*, void*) noexcept {}
void InputQueueDestroyedCallback(void*, void*) noexcept {}
void ActivityDestroyedCallback() {}

struct FakeInputEvent {
    int type = 0;
    bool preDispatched = false;
};

struct FakeInputQueue {
    std::vector<FakeInputEvent*> events;
    std::vector<FakeInputEvent*> finishedEvents;
    std::vector<int> handledValues;
    std::size_t nextEvent = 0;
    int preDispatchCount = 0;
};

int GetFakeInputEvent(void* context, void** outEvent) noexcept
{
    auto& queue = *static_cast<FakeInputQueue*>(context);
    if (queue.nextEvent >= queue.events.size()) {
        return -1;
    }
    *outEvent = queue.events[queue.nextEvent++];
    return 0;
}

int PreDispatchFakeInputEvent(void* context, void* event) noexcept
{
    auto& queue = *static_cast<FakeInputQueue*>(context);
    auto& inputEvent = *static_cast<FakeInputEvent*>(event);
    if (inputEvent.type != 1) {
        return 0;
    }
    ++queue.preDispatchCount;
    return inputEvent.preDispatched ? 1 : 0;
}

void FinishFakeInputEvent(void* context, void* event, int handled) noexcept
{
    auto& queue = *static_cast<FakeInputQueue*>(context);
    queue.finishedEvents.push_back(static_cast<FakeInputEvent*>(event));
    queue.handledValues.push_back(handled);
}

#if defined(__ANDROID__)
struct ActivityDestroySignal {
    std::promise<void> callbackStarted;
    int notificationCount = 0;
};

void SignalActivityDestroyed(void* context) noexcept
{
    auto& signal = *static_cast<ActivityDestroySignal*>(context);
    ++signal.notificationCount;
    signal.callbackStarted.set_value();
}
#endif

TEST(AndroidNativeActivityTests, CompletesApplicationOnlyOnce)
{
    NeoWPF::Platform::Android::AActivityLifecycle lifecycle;
    int finishCount = 0;

    EXPECT_TRUE(lifecycle.CompleteApplication(&CountFinish, &finishCount));
    EXPECT_FALSE(lifecycle.CompleteApplication(&CountFinish, &finishCount));
    EXPECT_EQ(finishCount, 1);
}

TEST(AndroidNativeActivityTests, DestructionPreventsActivityFinish)
{
    NeoWPF::Platform::Android::AActivityLifecycle lifecycle;
    int finishCount = 0;

    EXPECT_TRUE(lifecycle.MarkActivityDestroyed());
    EXPECT_FALSE(lifecycle.CompleteApplication(&CountFinish, &finishCount));
    EXPECT_EQ(finishCount, 0);
}

TEST(AndroidNativeActivityTests, CompletedApplicationCanBeDestroyedOnce)
{
    NeoWPF::Platform::Android::AActivityLifecycle lifecycle;
    int finishCount = 0;

    ASSERT_TRUE(lifecycle.CompleteApplication(&CountFinish, &finishCount));
    EXPECT_TRUE(lifecycle.MarkActivityDestroyed());
    EXPECT_FALSE(lifecycle.MarkActivityDestroyed());
    EXPECT_EQ(finishCount, 1);
}

TEST(AndroidNativeActivityTests, ActivityDestructionRemainsReportedAfterCompletion)
{
    NeoWPF::Platform::Android::AActivityLifecycle lifecycle;

    EXPECT_FALSE(lifecycle.IsActivityDestroying());
    ASSERT_TRUE(lifecycle.MarkActivityDestroyed());
    EXPECT_TRUE(lifecycle.IsActivityDestroying());
}

TEST(AndroidNativeActivityTests, SurfaceCreationAndResizeDispatchWindowDimensions)
{
    NeoWPF::Platform::Android::AActivityLifecycle lifecycle;
    SurfaceRecord record;
    int nativeWindow = 0;

    EXPECT_TRUE(lifecycle.DispatchSurfaceAvailable(
        &RecordSurfaceAvailable, &record, &nativeWindow, 640, 480));
    EXPECT_TRUE(lifecycle.DispatchSurfaceAvailable(
        &RecordSurfaceAvailable, &record, &nativeWindow, 800, 600));

    EXPECT_EQ(record.nativeWindow, &nativeWindow);
    EXPECT_EQ(record.width, 800u);
    EXPECT_EQ(record.height, 600u);
    EXPECT_EQ(record.availableCount, 2);
}

TEST(AndroidNativeActivityTests, SurfaceDestructionDispatchesUntilActivityIsDestroyed)
{
    NeoWPF::Platform::Android::AActivityLifecycle lifecycle;
    SurfaceRecord record;

    EXPECT_TRUE(lifecycle.DispatchSurfaceDestroyed(&RecordSurfaceDestroyed, &record));
    EXPECT_TRUE(lifecycle.MarkActivityDestroyed());
    EXPECT_FALSE(lifecycle.DispatchSurfaceDestroyed(&RecordSurfaceDestroyed, &record));
    EXPECT_EQ(record.destroyedCount, 1);
}

TEST(AndroidNativeActivityTests, ActivityDestructionRejectsFurtherSurfaceAvailability)
{
    NeoWPF::Platform::Android::AActivityLifecycle lifecycle;
    SurfaceRecord record;
    int nativeWindow = 0;

    ASSERT_TRUE(lifecycle.MarkActivityDestroyed());

    EXPECT_FALSE(lifecycle.DispatchSurfaceAvailable(
        &RecordSurfaceAvailable, &record, &nativeWindow, 640, 480));
    EXPECT_EQ(record.availableCount, 0);
}

TEST(AndroidNativeActivityTests, NativeWindowDispatchTransfersAcquiredReference)
{
    NeoWPF::Platform::Android::AActivityLifecycle lifecycle;
    NativeWindowRecord nativeWindow{1, 800, 600};
    SurfaceRecord surface;
    const NeoWPF::Platform::Android::ANativeWindowOperations operations{
        &AcquireWindow, &ReleaseWindow, &GetWindowWidth, &GetWindowHeight};

    EXPECT_TRUE(lifecycle.DispatchNativeWindow(
        &RecordSurfaceAvailable, &surface, &nativeWindow, operations));

    EXPECT_EQ(nativeWindow.referenceCount, 2);
    EXPECT_EQ(surface.nativeWindow, &nativeWindow);
    EXPECT_EQ(surface.width, 800u);
    EXPECT_EQ(surface.height, 600u);
}

TEST(AndroidNativeActivityTests, RejectedNativeWindowDispatchReleasesAcquiredReference)
{
    NeoWPF::Platform::Android::AActivityLifecycle lifecycle;
    NativeWindowRecord nativeWindow{1, 800, 600};
    SurfaceRecord surface;
    const NeoWPF::Platform::Android::ANativeWindowOperations operations{
        &AcquireWindow, &ReleaseWindow, &GetWindowWidth, &GetWindowHeight};
    ASSERT_TRUE(lifecycle.MarkActivityDestroyed());

    EXPECT_FALSE(lifecycle.DispatchNativeWindow(
        &RecordSurfaceAvailable, &surface, &nativeWindow, operations));

    EXPECT_EQ(nativeWindow.referenceCount, 1);
    EXPECT_EQ(surface.availableCount, 0);
}

/*
    @note
    A timed out worker may outlive its test after being detached. Replace this fallback with guaranteed shutdown when these tests are revisited.
*/
TEST(AndroidNativeActivityTests, ActivityDestructionWaitsForSurfaceCallback)
{
    auto lifecycle = std::make_shared<NeoWPF::Platform::Android::AActivityLifecycle>();
    auto callbackStarted = std::make_shared<std::promise<void>>();
    auto continueCallback = std::make_shared<std::promise<void>>();
    BlockingSurfaceRecord record{callbackStarted, continueCallback->get_future().share()};
    auto surfaceCallbackFinished = std::make_shared<std::promise<void>>();
    auto surfaceCallbackFinishedResult = surfaceCallbackFinished->get_future();
    auto destroyed = std::make_shared<std::promise<bool>>();
    auto destroyedResult = destroyed->get_future();

    std::thread surfaceCallback([lifecycle, record, surfaceCallbackFinished]() mutable {
        lifecycle->DispatchSurfaceAvailable(
            &BlockSurfaceAvailable, &record, nullptr, 0, 0);
        surfaceCallbackFinished->set_value();
    });
    const bool callbackStartedInTime =
        callbackStarted->get_future().wait_for(std::chrono::seconds(5)) ==
            std::future_status::ready;
    if (!callbackStartedInTime) {
        continueCallback->set_value();
        if (surfaceCallbackFinishedResult.wait_for(std::chrono::seconds(5)) ==
            std::future_status::ready) {
            surfaceCallback.join();
        } else {
            surfaceCallback.detach();
        }
        EXPECT_TRUE(callbackStartedInTime);
        return;
    }

    std::thread activityDestroy([lifecycle, destroyed] {
        destroyed->set_value(lifecycle->MarkActivityDestroyed());
    });
    const auto destructionDeadline = std::chrono::steady_clock::now() +
        std::chrono::seconds(5);
    while (!lifecycle->IsActivityDestroying() &&
           std::chrono::steady_clock::now() < destructionDeadline &&
           destroyedResult.wait_for(std::chrono::milliseconds(0)) ==
               std::future_status::timeout) {
        std::this_thread::yield();
    }
    const bool destructionStarted = lifecycle->IsActivityDestroying();
    EXPECT_TRUE(destructionStarted);
    const bool destructionWaitedForCallback =
        destroyedResult.wait_for(std::chrono::milliseconds(50)) ==
            std::future_status::timeout;

    continueCallback->set_value();
    const bool surfaceCallbackFinishedInTime =
        surfaceCallbackFinishedResult.wait_for(std::chrono::seconds(5)) ==
            std::future_status::ready;
    if (surfaceCallbackFinishedInTime) {
        surfaceCallback.join();
    } else {
        surfaceCallback.detach();
    }
    const bool activityDestroyFinishedInTime =
        destroyedResult.wait_for(std::chrono::seconds(5)) ==
            std::future_status::ready;
    const bool activityDestroyed = activityDestroyFinishedInTime && destroyedResult.get();
    if (activityDestroyFinishedInTime) {
        activityDestroy.join();
    } else {
        activityDestroy.detach();
    }

    EXPECT_TRUE(destructionWaitedForCallback);
    EXPECT_TRUE(surfaceCallbackFinishedInTime);
    EXPECT_TRUE(activityDestroyFinishedInTime);
    EXPECT_TRUE(activityDestroyed);
}

TEST(AndroidNativeActivityTests, CallbackRegistrationStoresAllNativeActivityHandlers)
{
    FakeActivityCallbacks callbacks;

    NeoWPF::Platform::Android::AActivityLifecycle::RegisterCallbacks(
        callbacks, &CreatedCallback, &ResizedCallback,
        &SurfaceDestroyedCallback, &InputQueueCreatedCallback,
        &InputQueueDestroyedCallback, &ActivityDestroyedCallback);

    EXPECT_EQ(callbacks.onNativeWindowCreated, &CreatedCallback);
    EXPECT_EQ(callbacks.onNativeWindowResized, &ResizedCallback);
    EXPECT_EQ(callbacks.onNativeWindowDestroyed, &SurfaceDestroyedCallback);
    EXPECT_EQ(callbacks.onInputQueueCreated, &InputQueueCreatedCallback);
    EXPECT_EQ(callbacks.onInputQueueDestroyed, &InputQueueDestroyedCallback);
    EXPECT_EQ(callbacks.onDestroy, &ActivityDestroyedCallback);
}

TEST(AndroidNativeActivityTests, InputQueueDrainCompletesUnconsumedEventsOnce)
{
    using NeoWPF::Platform::Android::AActivityInputQueueOperations;
    using NeoWPF::Platform::Android::ADrainUnhandledInputQueue;

    FakeInputEvent key{1, true};
    FakeInputEvent motion{2, false};
    FakeInputEvent unknown{3, false};
    FakeInputEvent regularKey{1, false};
    FakeInputQueue queue{{&key, &motion, &unknown, &regularKey}};
    const AActivityInputQueueOperations operations{
        &GetFakeInputEvent, &PreDispatchFakeInputEvent, &FinishFakeInputEvent};

    EXPECT_EQ(ADrainUnhandledInputQueue(&queue, operations), 3u);
    EXPECT_EQ(queue.preDispatchCount, 2);
    ASSERT_EQ(queue.finishedEvents.size(), 3u);
    EXPECT_EQ(queue.finishedEvents[0], &motion);
    EXPECT_EQ(queue.finishedEvents[1], &unknown);
    EXPECT_EQ(queue.finishedEvents[2], &regularKey);
    EXPECT_EQ(queue.handledValues, (std::vector<int>{0, 0, 0}));
}

#if defined(__ANDROID__)
TEST(AndroidNativeActivityTests, ProductionCallbacksDispatchWindowLifecycle)
{
    using NeoWPF::Platform::Android::AActivityHost;
    using NeoWPF::Platform::Android::ANativeWindowOperations;

    ANativeActivityCallbacks callbacks{};
    NeoWPF::Platform::Android::ARegisterActivityCallbacks(callbacks);
    ASSERT_NE(callbacks.onNativeWindowCreated, nullptr);
    ASSERT_NE(callbacks.onNativeWindowResized, nullptr);
    ASSERT_NE(callbacks.onNativeWindowDestroyed, nullptr);

    AActivityHost host;
    SurfaceRecord surface;
    NativeWindowRecord window{1, 640, 480};
    host.windowOperations = ANativeWindowOperations{
        &AcquireWindow, &ReleaseWindow, &GetWindowWidth, &GetWindowHeight};
    host.surfaceAvailable = &RecordSurfaceAvailable;
    host.surfaceAvailableContext = &surface;
    host.surfaceDestroyed = &RecordSurfaceDestroyed;
    host.surfaceDestroyedContext = &surface;

    ANativeActivity activity{};
    activity.instance = &host;
    auto* const nativeWindow = reinterpret_cast<ANativeWindow*>(&window);
    callbacks.onNativeWindowCreated(&activity, nativeWindow);
    window.width = 800;
    window.height = 600;
    callbacks.onNativeWindowResized(&activity, nativeWindow);
    callbacks.onNativeWindowDestroyed(&activity, nativeWindow);

    EXPECT_EQ(window.referenceCount, 3);
    EXPECT_EQ(surface.nativeWindow, &window);
    EXPECT_EQ(surface.width, 800u);
    EXPECT_EQ(surface.height, 600u);
    EXPECT_EQ(surface.availableCount, 2);
    EXPECT_EQ(surface.destroyedCount, 1);

    ReleaseWindow(&window);
    ReleaseWindow(&window);
    EXPECT_EQ(window.referenceCount, 1);
}

TEST(AndroidNativeActivityTests, ProductionDestroyCallbackJoinsAndReleasesHost)
{
    using NeoWPF::Platform::Android::AActivityHost;

    ANativeActivityCallbacks callbacks{};
    NeoWPF::Platform::Android::ARegisterActivityCallbacks(callbacks);
    ASSERT_NE(callbacks.onDestroy, nullptr);

    auto applicationStarted = std::make_shared<std::promise<void>>();
    auto allowApplicationExit = std::make_shared<std::promise<void>>();
    const auto allowApplicationExitSignal = allowApplicationExit->get_future().share();
    auto applicationFinished = std::make_shared<std::promise<void>>();
    auto applicationFinishedResult = applicationFinished->get_future();
    auto destroySignal = std::make_shared<ActivityDestroySignal>();
    auto* const host = new AActivityHost();
    host->activityDestroyed = &SignalActivityDestroyed;
    host->activityDestroyedContext = destroySignal.get();
    host->applicationThread = std::thread([
        applicationStarted, allowApplicationExitSignal, applicationFinished] {
        applicationStarted->set_value();
        allowApplicationExitSignal.wait();
        applicationFinished->set_value();
    });
    auto applicationStartedResult = applicationStarted->get_future();

    auto activity = std::make_shared<ANativeActivity>();
    activity->instance = host;
    const bool applicationStartedInTime =
        applicationStartedResult.wait_for(std::chrono::seconds(5)) ==
            std::future_status::ready;
    if (!applicationStartedInTime) {
        allowApplicationExit->set_value();
        if (applicationFinishedResult.wait_for(std::chrono::seconds(5)) ==
            std::future_status::ready) {
            host->applicationThread.join();
            delete host;
            activity->instance = nullptr;
        } else {
            host->applicationThread.detach();
        }
        EXPECT_TRUE(applicationStartedInTime);
        return;
    }

    auto destroyCallbackFinished = std::make_shared<std::promise<void>>();
    auto destroyCallbackFinishedResult = destroyCallbackFinished->get_future();
    auto onDestroy = callbacks.onDestroy;
    std::thread destroyActivity([activity, destroySignal, destroyCallbackFinished,
                                 onDestroy] {
        onDestroy(activity.get());
        destroyCallbackFinished->set_value();
    });

    const bool destroyCallbackStartedInTime =
        destroySignal->callbackStarted.get_future().wait_for(std::chrono::seconds(5)) ==
            std::future_status::ready;
    const bool destroyCallbackWaitedForApplication =
        destroyCallbackFinishedResult.wait_for(std::chrono::milliseconds(50)) ==
            std::future_status::timeout;
    allowApplicationExit->set_value();
    const bool destroyCallbackFinishedInTime =
        destroyCallbackFinishedResult.wait_for(std::chrono::seconds(5)) ==
            std::future_status::ready;
    if (destroyCallbackFinishedInTime) {
        destroyActivity.join();
    } else {
        destroyActivity.detach();
    }

    EXPECT_TRUE(destroyCallbackStartedInTime);
    EXPECT_TRUE(destroyCallbackWaitedForApplication);
    EXPECT_TRUE(destroyCallbackFinishedInTime);
    if (destroyCallbackFinishedInTime) {
        EXPECT_EQ(activity->instance, nullptr);
        EXPECT_EQ(destroySignal->notificationCount, 1);
    }
}
#endif

}