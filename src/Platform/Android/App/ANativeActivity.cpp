#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Platform/Android/AActivityHost.h>
#include <NeoWPF/Platform/Android/AActivityLifecycle.h>
#include <NeoWPF/Platform/Android/APlatform.h>
#include <NeoWPF/Platform/PlatformRegistry.h>

#include <android/log.h>
#include <android/input.h>
#include <android/native_activity.h>
#include <android/native_window.h>

#include <exception>
#include <memory>
#include <thread>
#include <unistd.h>

int NeoWPFAndroidApplicationMain();

namespace NeoWPF::Platform::Android {
namespace {

AWindowing* GetWindowing() noexcept
{
    auto* platform = dynamic_cast<APlatform*>(NeoWPF::Platform::CurrentPlatform());
    return platform ? static_cast<AWindowing*>(&platform->Windowing()) : nullptr;
}

void FinishActivity(void* context) noexcept
{
    ANativeActivity_finish(static_cast<ANativeActivity*>(context));
}

void QueueSurfaceAvailable(void* context, void* nativeWindow,
                            unsigned int width, unsigned int height) noexcept
{
    static_cast<AWindowing*>(context)->QueueSurfaceAvailable(
        static_cast<ANativeWindow*>(nativeWindow), width, height);
}

void QueueSurfaceDestroyed(void* context) noexcept
{
    static_cast<AWindowing*>(context)->QueueSurfaceDestroyed();
}

void QueueActivityDestroyed(void* context) noexcept
{
    static_cast<AWindowing*>(context)->QueueActivityDestroyed();
}

void QueueInputQueueCreated(void* context, void* inputQueue) noexcept
{
    static_cast<AWindowing*>(context)->QueueInputQueueCreated(
        static_cast<AInputQueue*>(inputQueue));
}

void QueueInputQueueDestroyed(void* context, void* inputQueue) noexcept
{
    static_cast<AWindowing*>(context)->QueueInputQueueDestroyed(
        static_cast<AInputQueue*>(inputQueue));
}

void AcquireNativeWindow(void* nativeWindow) noexcept
{
    ANativeWindow_acquire(static_cast<ANativeWindow*>(nativeWindow));
}

void ReleaseNativeWindow(void* nativeWindow) noexcept
{
    ANativeWindow_release(static_cast<ANativeWindow*>(nativeWindow));
}

int GetNativeWindowWidth(void* nativeWindow) noexcept
{
    return ANativeWindow_getWidth(static_cast<ANativeWindow*>(nativeWindow));
}

int GetNativeWindowHeight(void* nativeWindow) noexcept
{
    return ANativeWindow_getHeight(static_cast<ANativeWindow*>(nativeWindow));
}

constexpr ANativeWindowOperations WindowOperations{
    &AcquireNativeWindow,
    &ReleaseNativeWindow,
    &GetNativeWindowWidth,
    &GetNativeWindowHeight,
};

void CleanupAfterFailure(AActivityHost& host) noexcept
{
    try {
        NeoWPF::Core::Application::Shutdown();
    } catch (const std::exception& exception) {
        NeoWPF::Diagnostics::LogError("Android application shutdown failed: %s", exception.what());
    } catch (...) {
        NeoWPF::Diagnostics::LogError("Android application shutdown failed");
    }

    if (auto* const windowing = GetWindowing()) {
        windowing->Stop();
    }
    host.lifecycle.CompleteApplication(&FinishActivity, host.activity);
}

void RunApplication(AActivityHost* host) noexcept
{
    try {
        (void)NeoWPFAndroidApplicationMain();
        host->lifecycle.CompleteApplication(&FinishActivity, host->activity);
    } catch (const std::exception& exception) {
        NeoWPF::Diagnostics::LogError("Android application failed: %s", exception.what());
        CleanupAfterFailure(*host);
    } catch (...) {
        NeoWPF::Diagnostics::LogError("Android application failed with an unknown exception");
        CleanupAfterFailure(*host);
    }
}

void OnWindowCreated(ANativeActivity* activity, ANativeWindow* window) noexcept
{
    auto* const host = static_cast<AActivityHost*>(activity->instance);
    if (!host || !window) {
        return;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
        NeoWPF::Diagnostics::Level::Trace,
        "[Android/Surface] callback created: activity=%p host=%p nativeWindow=%p",
        activity, host, window);

    host->lifecycle.DispatchNativeWindow(
        host->surfaceAvailable, host->surfaceAvailableContext,
        window, host->windowOperations);
}

void OnWindowResized(ANativeActivity* activity, ANativeWindow* window) noexcept
{
    auto* const host = static_cast<AActivityHost*>(activity->instance);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
        NeoWPF::Diagnostics::Level::Trace,
        "[Android/Surface] callback resized: activity=%p host=%p nativeWindow=%p",
        activity, host, window);
    OnWindowCreated(activity, window);
}

void OnWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) noexcept
{
    auto* const host = static_cast<AActivityHost*>(activity->instance);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
        NeoWPF::Diagnostics::Level::Trace,
        "[Android/Surface] callback destroyed: activity=%p host=%p nativeWindow=%p",
        activity, host, window);
    if (host) {
        host->lifecycle.DispatchSurfaceDestroyed(
            host->surfaceDestroyed, host->surfaceDestroyedContext);
    }
}

void OnInputQueueCreated(ANativeActivity* activity, AInputQueue* inputQueue) noexcept
{
    auto* const host = static_cast<AActivityHost*>(activity->instance);
    const bool dispatched = host && host->lifecycle.DispatchInputQueueCreated(
        host->inputQueueCreated, host->inputQueueCreatedContext, inputQueue);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
        NeoWPF::Diagnostics::Level::Debug,
        "T1 OnInputQueueCreated queue=%p dispatched=%d tid=%d",
        inputQueue, dispatched, gettid());
}

void OnInputQueueDestroyed(ANativeActivity* activity, AInputQueue* inputQueue) noexcept
{
    auto* const host = static_cast<AActivityHost*>(activity->instance);
    if (host) {
        host->lifecycle.DispatchInputQueueDestroyed(
            host->inputQueueDestroyed, host->inputQueueDestroyedContext, inputQueue);
    }
}

void OnDestroy(ANativeActivity* activity) noexcept
{
    auto* const host = static_cast<AActivityHost*>(activity->instance);
    if (host) {
        host->lifecycle.MarkActivityDestroyed();
    }
    if (host) {
        if (host->activityDestroyed) {
            host->activityDestroyed(host->activityDestroyedContext);
        }
    }

    if (host) {
        if (host->applicationThread.joinable()) {
            host->applicationThread.join();
        }
        delete host;
        activity->instance = nullptr;
    }
}

}

void ARegisterActivityCallbacks(ANativeActivityCallbacks& callbacks) noexcept
{
    AActivityLifecycle::RegisterCallbacks(
        callbacks, &OnWindowCreated, &OnWindowResized,
        &OnWindowDestroyed, &OnInputQueueCreated,
        &OnInputQueueDestroyed, &OnDestroy);
}

}

extern "C" void ANativeActivity_onCreate(
    ANativeActivity* activity, void*, std::size_t)
{
    if (!activity || !activity->callbacks) {
        return;
    }

    try {
        NeoWPF::Diagnostics::SetOutputHandler([](const char* message) noexcept {
            __android_log_write(ANDROID_LOG_INFO, "NeoWPF", message);
        });

        auto host = std::make_unique<NeoWPF::Platform::Android::AActivityHost>();
        host->activity = activity;
        NeoWPF::Platform::RegisterPlatform(
            std::make_unique<NeoWPF::Platform::Android::APlatform>(
                activity->internalDataPath ? activity->internalDataPath : ""));
        auto* const windowing = NeoWPF::Platform::Android::GetWindowing();
        host->windowOperations = NeoWPF::Platform::Android::WindowOperations;
        host->surfaceAvailable = &NeoWPF::Platform::Android::QueueSurfaceAvailable;
        host->surfaceAvailableContext = windowing;
        host->surfaceDestroyed = &NeoWPF::Platform::Android::QueueSurfaceDestroyed;
        host->surfaceDestroyedContext = windowing;
        host->inputQueueCreated = &NeoWPF::Platform::Android::QueueInputQueueCreated;
        host->inputQueueCreatedContext = windowing;
        host->inputQueueDestroyed = &NeoWPF::Platform::Android::QueueInputQueueDestroyed;
        host->inputQueueDestroyedContext = windowing;
        host->activityDestroyed = &NeoWPF::Platform::Android::QueueActivityDestroyed;
        host->activityDestroyedContext = windowing;
        activity->instance = host.get();
        NeoWPF::Platform::Android::ARegisterActivityCallbacks(*activity->callbacks);
        host->applicationThread = std::thread(
            &NeoWPF::Platform::Android::RunApplication, host.get());
        host.release();
    } catch (const std::exception& exception) {
        activity->instance = nullptr;
        NeoWPF::Diagnostics::LogError("Failed to start Android host: %s", exception.what());
        ANativeActivity_finish(activity);
    } catch (...) {
        activity->instance = nullptr;
        NeoWPF::Diagnostics::LogError("Failed to start Android host");
        ANativeActivity_finish(activity);
    }
}