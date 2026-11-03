/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>
#include <condition_variable>
#include <mutex>

namespace NeoWPF::Platform::Android {

/*
    @summary
    Provides native-window operations used by Activity lifecycle dispatch.

    @internal
*/
struct ANativeWindowOperations {
    void (*acquire)(void*) noexcept = nullptr;
    void (*release)(void*) noexcept = nullptr;
    int (*width)(void*) noexcept = nullptr;
    int (*height)(void*) noexcept = nullptr;
};

/*
    @summary
    Serializes Android application completion against Activity destruction.

    @internal
*/
class AActivityLifecycle final {
public:
    using SurfaceAvailableCallback = void (*)(void*, void*, unsigned int, unsigned int) noexcept;
    using SurfaceDestroyedCallback = void (*)(void*) noexcept;
    using InputQueueCallback = void (*)(void*, void*) noexcept;

    /*
        @summary
        Registers the NativeActivity window, input queue, and destruction callbacks.

        @param callbacks
        NativeActivity callback table.

        @param windowCreated
        Window creation callback.

        @param windowResized
        Window resize callback.

        @param windowDestroyed
        Window destruction callback.

        @param inputQueueCreated
        Input queue creation callback.

        @param inputQueueDestroyed
        Input queue destruction callback.

        @param activityDestroyed
        Activity destruction callback.

        @internal
    */
    template <typename CallbackTable, typename WindowCreated,
              typename WindowResized, typename WindowDestroyed,
              typename InputQueueCreated, typename InputQueueDestroyed,
              typename ActivityDestroyed>
    static void RegisterCallbacks(CallbackTable& callbacks,
                                  WindowCreated windowCreated,
                                  WindowResized windowResized,
                                  WindowDestroyed windowDestroyed,
                                  InputQueueCreated inputQueueCreated,
                                  InputQueueDestroyed inputQueueDestroyed,
                                  ActivityDestroyed activityDestroyed) noexcept
    {
        callbacks.onNativeWindowCreated = windowCreated;
        callbacks.onNativeWindowResized = windowResized;
        callbacks.onNativeWindowDestroyed = windowDestroyed;
        callbacks.onInputQueueCreated = inputQueueCreated;
        callbacks.onInputQueueDestroyed = inputQueueDestroyed;
        callbacks.onDestroy = activityDestroyed;
    }

    /*
        @summary
        Dispatches input queue creation while the Activity is active.

        @param callback
        Receives the opaque context and borrowed input queue.

        @param context
        Opaque callback context.

        @param inputQueue
        Borrowed NativeActivity input queue.

        @returns
        True when the callback was invoked.

        @internal
    */
    bool DispatchInputQueueCreated(InputQueueCallback callback, void* context,
                                   void* inputQueue) noexcept;

    /*
        @summary
        Dispatches input queue destruction until the Activity is destroyed.

        @param callback
        Receives the opaque context and borrowed input queue.

        @param context
        Opaque callback context.

        @param inputQueue
        Borrowed NativeActivity input queue.

        @returns
        True when the callback was invoked.

        @internal
    */
    bool DispatchInputQueueDestroyed(InputQueueCallback callback, void* context,
                                     void* inputQueue) noexcept;

    /*
        @summary
        Acquires and dispatches a NativeActivity window, releasing it if rejected.

        @param callback
        Receives the context, native window, and dimensions.

        @param context
        Opaque callback context.

        @param nativeWindow
        Borrowed NativeActivity window.

        @param operations
        Native window acquisition, release, and dimension operations.

        @returns
        True when ownership transfers to the callback.

        @internal
    */
    bool DispatchNativeWindow(SurfaceAvailableCallback callback, void* context,
                              void* nativeWindow,
                              const ANativeWindowOperations& operations) noexcept;

    /*
        @summary
        Dispatches a newly available surface while the Activity is active.

        @param callback
        Receives the context, native window, and dimensions.

        @param context
        Opaque callback context.

        @param nativeWindow
        Native surface handle.

        @param width
        Surface width in pixels.

        @param height
        Surface height in pixels.

        @returns
        True when the callback was invoked.

        @internal
    */
    bool DispatchSurfaceAvailable(SurfaceAvailableCallback callback, void* context,
                                  void* nativeWindow, unsigned int width,
                                  unsigned int height) noexcept;

    /*
        @summary
        Dispatches surface teardown until the Activity is destroyed.

        @param callback
        Receives the opaque callback context.

        @param context
        Opaque callback context.

        @returns
        True when the callback was invoked.

        @internal
    */
    bool DispatchSurfaceDestroyed(SurfaceDestroyedCallback callback,
                                  void* context) noexcept;

    /*
        @summary
        Runs the Activity completion callback once unless destruction has started.

        @param finishActivity
        Callback that finishes the Activity.

        @param context
        Context passed to the completion callback.

        @returns
        True when the callback was invoked.

        @internal
    */
    bool CompleteApplication(void (*finishActivity)(void*) noexcept,
                             void* context) noexcept;

    /*
        @summary
        Marks Activity destruction and rejects duplicate notifications.

        @returns
        True when this call changes the lifecycle state.

        @internal
    */
    bool MarkActivityDestroyed() noexcept;

    /*
        @summary
        Indicates whether Activity destruction has started.

        @returns
        True when destruction is in progress or complete.

        @internal
    */
    bool IsActivityDestroying() const noexcept;

private:
    enum class State {
        Active,
        ApplicationCompleted,
        ActivityDestroying,
        ActivityDestroyed,
    };

    /*
        @summary
        Starts a lifecycle callback unless the Activity is destroying.

        @param requireActive
        True when only an active Activity can start the callback.

        @returns
        True when the callback is counted as in flight.

        @internal
    */
    bool BeginCallback(bool requireActive) noexcept;

    /*
        @summary
        Marks a lifecycle callback complete and wakes waiting destruction.

        @internal
    */
    void EndCallback() noexcept;

    mutable std::mutex m_mutex;
    std::condition_variable m_callbacksFinished;
    std::size_t m_inFlightCallbacks = 0;
    State m_state = State::Active;
};

}