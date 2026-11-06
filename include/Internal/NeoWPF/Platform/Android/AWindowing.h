/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowing.h>

#include <android/input.h>
#include <android/looper.h>
#include <android/native_window.h>

#include <atomic>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

namespace NeoWPF::Platform::Android {

class AWindow;

/*
    @summary
    Converts an Android keycode to a NeoWPF key code.

    @param keyCode
    Android keycode.

    @returns
    The matching key code, or `Key::None` when it is not mapped.

    @internal
*/
int TranslateAndroidKeyCode(int keyCode) noexcept;

/*
    @summary
    Provides the single Android host window and pumps Activity events.

    @internal
*/
class AWindowing final : public NeoWPF::Contracts::IWindowing {
public:
    ~AWindowing() override;

    /*
        @summary
        Creates the single stable Android window record.

        @param title
        Window title, ignored by Android.

        @param width
        Requested width, ignored until the surface reports its size.

        @param height
        Requested height, ignored until the surface reports its size.

        @param windowHost
        Optional UI content host.

        @returns
        The stable window handle, or null if one already exists.

        @internal
    */
    void* CreateWindow(const wchar_t* title, int width, int height,
                       std::shared_ptr<NeoWPF::UI::Window> windowHost = nullptr) override;

    /*
        @summary
        Fills Vulkan surface information for a live Android surface.

        @param nativeWindowHandle
        Stable Android window handle.

        @param outInfo
        Receives native Vulkan surface information.

        @returns
        True when a surface is currently available.

        @internal
    */
    bool GetNativeWindowInfo(void* nativeWindowHandle,
                             NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept override;

    /*
        @summary
        Indicates whether the Activity still owns the window.

        @param nativeWindowHandle
        Stable Android window handle.

        @internal
    */
    bool IsWindowAlive(void* nativeWindowHandle) const noexcept override;

    /*
        @summary
        Reads the most recently reported surface size.

        @param nativeWindowHandle
        Stable Android window handle.

        @param outWidth
        Receives the surface width.

        @param outHeight
        Receives the surface height.

        @internal
    */
    void GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth,
                       unsigned int& outHeight) const noexcept override;

    /*
        @summary
        Android surface dimensions are controlled by the Activity.

        @internal
    */
    void ResizeWindow(void* nativeWindowHandle, int width, int height) noexcept override;
    /*
        @summary
        Android visibility is controlled by the Activity.

        @internal
    */
    void ShowWindow(void* nativeWindowHandle) noexcept override;
    /*
        @summary
        Android visibility is controlled by the Activity.

        @internal
    */
    void HideWindow(void* nativeWindowHandle) noexcept override;

    /*
        @summary
        Closes and removes the supplied Android window.

        @param nativeWindowHandle
        Stable Android window handle.

        @internal
    */
    void CloseWindow(void* nativeWindowHandle) noexcept override;

    /*
        @summary
        Wakes the Android application event pump.

        @internal
    */
    void InvalidateWindow(void* nativeWindowHandle) noexcept override;

    /*
        @summary
        Processes queued window and input-queue lifecycle events.

        @param timeout
        Maximum time to wait for an event.

        @returns
        True while the Activity and window remain alive.

        @internal
    */
    bool PumpMessages(std::chrono::milliseconds timeout) override;

    /*
        @summary
        Returns the last requested application exit code.

        @internal
    */
    int ExitCode() const noexcept override;
    /*
        @summary
        Requests that the Android application loop stop.

        @internal
    */
    void RequestQuit(int exitCode) noexcept override;
    /*
        @summary
        Logs an Android platform error.

        @internal
    */
    void ShowFatalError(const std::wstring& title, const std::wstring& message) noexcept override;
    /*
        @summary
        Console visibility is unsupported on Android.

        @internal
    */
    void SetConsoleVisible(bool visible) noexcept override;
    /*
        @summary
        Console visibility is unsupported on Android.

        @internal
    */
    bool IsConsoleVisible() const noexcept override;

    /*
        @summary
        Queues an acquired surface reference for the application thread to own.

        @param nativeWindow
        Acquired native surface reference; ownership transfers to this object.

        @param width
        Surface width in pixels.

        @param height
        Surface height in pixels.

        @internal
    */
    void QueueSurfaceAvailable(ANativeWindow* nativeWindow,
                               unsigned int width, unsigned int height) noexcept;

    /*
        @summary
        Queues surface teardown and waits for render use to stop.

        @internal
    */
    void QueueSurfaceDestroyed() noexcept;

    /*
        @summary
        Queues a borrowed input queue for attachment to the application looper.

        @param inputQueue
        NativeActivity input queue, valid until its destroy callback returns.

        @internal
    */
    void QueueInputQueueCreated(AInputQueue* inputQueue) noexcept;

    /*
        @summary
        Detaches an input queue before its destroy callback returns.

        @param inputQueue
        NativeActivity input queue being destroyed.

        @internal
    */
    void QueueInputQueueDestroyed(AInputQueue* inputQueue) noexcept;

    /*
        @summary
        Signals that the Activity is being destroyed.

        @internal
    */
    void QueueActivityDestroyed() noexcept;

    /*
        @summary
        Indicates whether the window has received a native surface.

        @internal
    */
    bool HasSurface() const noexcept;

    /*
        @summary
        Pumps queued events until a surface arrives, the Activity stops, or timeout expires.

        @param timeout
        Maximum time to wait for the first surface.

        @returns
        True when a surface becomes available.

        @internal
    */
    bool WaitForSurface(std::chrono::seconds timeout);

    /*
        @summary
        Stops event processing and releases surfaces after rendering has stopped.

        @internal
    */
    void Stop() noexcept;

private:
    enum class EventType {
        SurfaceAvailable,
        SurfaceDestroyed,
        InputQueueCreated,
        InputQueueDestroyed,
    };

    struct Event {
        EventType type = EventType::SurfaceDestroyed;
        ANativeWindow* nativeWindow = nullptr;
        AInputQueue* inputQueue = nullptr;
        unsigned int width = 0;
        unsigned int height = 0;
        std::shared_ptr<std::promise<void>> completion;
    };

    /*
        @summary
        Finds a mutable Android window by its stable handle.

        @internal
    */
    AWindow* FindWindow(void* nativeWindowHandle) noexcept;
    /*
        @summary
        Finds a const Android window by its stable handle.

        @internal
    */
    const AWindow* FindWindow(void* nativeWindowHandle) const noexcept;
    /*
        @summary
        Processes queued Android Activity events.

        @internal
    */
    void ProcessEvents() noexcept;
    /*
        @summary
        Applies a newly available Android surface to the window.

        @internal
    */
    void ProcessSurfaceAvailable(Event& event) noexcept;
    /*
        @summary
        Releases the current Android surface after rendering stops.

        @internal
    */
    void ProcessSurfaceDestroyed() noexcept;
    /*
        @summary
        Attaches an input queue to the application thread's looper.

        @internal
    */
    void ProcessInputQueueEvent(Event& event) noexcept;
    /*
        @summary
        Processes a NativeActivity input event.

        @internal
    */
    int HandleInputQueueEvent(void* event) noexcept;
    /*
        @summary
        Translates a NativeActivity key event into framework keyboard input.

        @internal
    */
    int HandleKeyInputEvent(AInputEvent* event) noexcept;
    /*
        @summary
        Translates a NativeActivity motion event into framework pointer input.

        @internal
    */
    int HandlePointerInputEvent(AInputEvent* event) noexcept;
    /*
        @summary
        Detaches the current input queue from the application looper.

        @internal
    */
    void StopInputQueue() noexcept;
    /*
        @summary
        Wakes the application thread's looper for queued events.

        @internal
    */
    void WakeEventPump() noexcept;
    /*
        @summary
        Drains input events signaled on the application thread's looper.

        @internal
    */
    static int InputQueueLooperCallback(int fd, int events, void* context) noexcept;

    std::vector<std::unique_ptr<AWindow>> m_windows;
    std::atomic<bool> m_activityAlive{true};
    std::atomic<int> m_exitCode{0};
    mutable std::mutex m_eventMutex;
    std::deque<Event> m_events;
    ALooper* m_looper = nullptr;
    AInputQueue* m_inputQueue = nullptr;
    bool m_stopped = false;
};

}