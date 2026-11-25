/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowing.h>
#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>
#include <fractional-scale-v1-client-protocol.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace NeoWPF::Platform::Linux {
class WlWindow;
class WlClipboard;
}

namespace NeoWPF::Platform::Linux {

class WlWindowing final : public NeoWPF::Contracts::IWindowing {
public:
    WlWindowing() = default;
    ~WlWindowing();

    WlWindowing(const WlWindowing&) = delete;
    WlWindowing& operator=(const WlWindowing&) = delete;

    void* CreateWindow(
        const wchar_t* title,
        int width,
        int height,
        std::shared_ptr<NeoWPF::UI::Window> windowHost = nullptr) override;
    bool GetNativeWindowInfo(void* nativeWindowHandle, NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept override;
    bool IsWindowAlive(void* nativeWindowHandle) const noexcept override;
    void GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth, unsigned int& outHeight) const noexcept override;
    /*
        @summary
        Returns the scale from Wayland surface coordinates to device-independent units.

        @param nativeWindowHandle
        Native window handle.

        @param outScaleFactor
        Receives the physical-pixel scale.

        @returns
        True when the window is configured.

        @internal
    */
    bool GetWindowScaleFactor(void* nativeWindowHandle,
                              double& outScaleFactor) const noexcept override;
    bool GetWindowInsets(void* nativeWindowHandle,
                         NeoWPF::Contracts::WindowInsets& outSafeArea,
                         NeoWPF::Contracts::WindowInsets& outKeyboard) const noexcept override;
    bool GetInputProfile(void* nativeWindowHandle,
                         NeoWPF::Contracts::InputProfile& outProfile) const noexcept override;
    void ResizeWindow(void* nativeWindowHandle, int width, int height) noexcept override;
    void ShowWindow(void* nativeWindowHandle) noexcept override;
    void HideWindow(void* nativeWindowHandle) noexcept override;
    void CloseWindow(void* nativeWindowHandle) noexcept override;
    void InvalidateWindow(void* nativeWindowHandle) noexcept override;
    bool PumpMessages(std::chrono::milliseconds timeout) override;
    int ExitCode() const noexcept override;
    void RequestQuit(int exitCode) noexcept override;
    void ShowFatalError(const std::wstring& title, const std::wstring& message) noexcept override;
    void SetConsoleVisible(bool visible) noexcept override;
    bool IsConsoleVisible() const noexcept override;

    bool Connect();
    void Disconnect() noexcept;
    bool IsConnected() const noexcept;

    wl_display* Display() const noexcept;
    wl_compositor* Compositor() const noexcept;
    wl_shm* Shm() const noexcept;
    wl_seat* Seat() const noexcept;
    wl_data_device_manager* DataDeviceManager() const noexcept;
    /*
        @summary
        Returns the integer scale reported for an output.

        @param output
        Wayland output.

        @returns
        The output scale, or zero when the output is unknown.

        @internal
    */
    int GetOutputScale(wl_output* output) const noexcept;
    void AttachClipboard(WlClipboard* clipboard) noexcept;
    void DetachClipboard(WlClipboard* clipboard) noexcept;
    void RegisterInputSerial(std::uint32_t serial) noexcept;
    std::uint32_t LastInputSerial() const noexcept;
    xdg_wm_base* XdgWmBase() const noexcept;
    bool HasXdgShell() const noexcept;
    wp_fractional_scale_manager_v1* FractionalScaleManager() const noexcept;

    static void HandleGlobal(void* data, wl_registry* registry, std::uint32_t name,
                             const char* interface, std::uint32_t version);
    static void HandleGlobalRemove(void* data, wl_registry* registry, std::uint32_t name);
    /*
        @summary
        Updates the integer scale reported by a Wayland output.

        @internal
    */
    static void HandleOutputScale(void* data, wl_output* output, int32_t scale);

private:
    struct OutputState {
        std::uint32_t name = 0;
        wl_output* output = nullptr;
        int scale = 1;
    };

    WlWindow* FindWindow(void* nativeWindowHandle) noexcept;
    const WlWindow* FindWindow(void* nativeWindowHandle) const noexcept;

    wl_display* m_display = nullptr;
    wl_registry* m_registry = nullptr;
    wl_compositor* m_compositor = nullptr;
    wl_shm* m_shm = nullptr;
    wl_seat* m_seat = nullptr;
    wl_data_device_manager* m_dataDeviceManager = nullptr;
    xdg_wm_base* m_xdgWmBase = nullptr;
    wp_fractional_scale_manager_v1* m_fractionalScaleManager = nullptr;
    std::vector<OutputState> m_outputs;
    bool m_hasXdgShell = false;
    WlClipboard* m_clipboard = nullptr;
    std::uint32_t m_lastInputSerial = 0;
    std::vector<std::unique_ptr<WlWindow>> m_windows;
    int m_exitCode = 0;
    bool m_consoleVisible = false;
};

}