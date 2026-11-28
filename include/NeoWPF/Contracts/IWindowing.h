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

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

namespace NeoWPF::UI {
class Window;
}

namespace NeoWPF::Contracts {

struct WindowInsets {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
};

enum class PointerPrecision {
    Fine,
    Coarse
};

struct InputProfile {
    PointerPrecision primaryPointer = PointerPrecision::Fine;
    bool canHover = true;
    bool hasTouch = false;
};

enum class NativeWindowSystem {
    Unknown,
    Win32,
    X11,
    Wayland,
    Android,
};

enum class SurfaceAlpha {
    Opaque,
    Premultiplied,
};

using VulkanSurfaceCreateCallback = bool (*)(
    void* userData,
    void* instance,
    void* window,
    void* display,
    void* outSurface) noexcept;

struct NativeWindowInfo {
    NativeWindowSystem system = NativeWindowSystem::Unknown;
    SurfaceAlpha surfaceAlpha = SurfaceAlpha::Opaque;
    void* window = nullptr;
    void* display = nullptr;
    const char* const* vulkanInstanceExtensions = nullptr;
    std::size_t vulkanInstanceExtensionCount = 0;
    VulkanSurfaceCreateCallback createVulkanSurface = nullptr;
    void* vulkanSurfaceUserData = nullptr;
};

class IWindowing {
public:
    virtual ~IWindowing() = default;

    virtual void* CreateWindow(
        const wchar_t* title,
        int width,
        int height,
        std::shared_ptr<NeoWPF::UI::Window> windowHost = nullptr) = 0;
    virtual bool GetNativeWindowInfo(void* nativeWindowHandle, NativeWindowInfo& outInfo) const noexcept = 0;
    virtual bool IsWindowAlive(void* nativeWindowHandle) const noexcept = 0;
    virtual void GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth, unsigned int& outHeight) const noexcept = 0;
    /*
        @summary
        Returns the physical-pixel scale for a live window.

        @param nativeWindowHandle
        Native window handle.

        @param outScaleFactor
        Receives the number of physical pixels per device-independent unit.

        @returns
        True when the window scale is available and positive.

        @internal
    */
    virtual bool GetWindowScaleFactor(void* nativeWindowHandle,
                                     double& outScaleFactor) const noexcept = 0;
    /*
        @summary
        Returns physical-pixel window insets for the native window.
    */
    virtual bool GetWindowInsets(void* nativeWindowHandle,
                                 WindowInsets& outSafeArea,
                                 WindowInsets& outKeyboard) const noexcept = 0;
    /*
        @summary
        Returns the static input profile for the native window.
    */
    virtual bool GetInputProfile(void* nativeWindowHandle,
                                 InputProfile& outProfile) const noexcept = 0;
    virtual void ResizeWindow(void* nativeWindowHandle, int width, int height) noexcept = 0;
    virtual void ShowWindow(void* nativeWindowHandle) noexcept = 0;
    virtual void HideWindow(void* nativeWindowHandle) noexcept = 0;
    virtual void CloseWindow(void* nativeWindowHandle) noexcept = 0;
    virtual void InvalidateWindow(void* nativeWindowHandle) noexcept = 0;
    virtual bool PumpMessages(std::chrono::milliseconds timeout) = 0;
    virtual int ExitCode() const noexcept = 0;
    virtual void RequestQuit(int exitCode) noexcept = 0;
    virtual void ShowFatalError(const std::wstring& title, const std::wstring& message) noexcept = 0;
    virtual void SetConsoleVisible(bool visible) noexcept = 0;
    virtual bool IsConsoleVisible() const noexcept = 0;
};

}
