/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowing.h>
#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace NeoWPF::Platform::Linux {
class LWindow;
class LClipboard;
}

namespace NeoWPF::Platform::Linux {

class LWindowing final : public NeoWPF::Contracts::IWindowing {
public:
    LWindowing() = default;
    ~LWindowing();

    LWindowing(const LWindowing&) = delete;
    LWindowing& operator=(const LWindowing&) = delete;

    void* CreateWindow(
        const wchar_t* title,
        int width,
        int height,
        std::shared_ptr<NeoWPF::UI::Window> windowHost = nullptr) override;
    bool GetNativeWindowInfo(void* nativeWindowHandle, NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept override;
    bool IsWindowAlive(void* nativeWindowHandle) const noexcept override;
    void GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth, unsigned int& outHeight) const noexcept override;
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
    void AttachClipboard(LClipboard* clipboard) noexcept;
    void DetachClipboard(LClipboard* clipboard) noexcept;
    void RegisterInputSerial(std::uint32_t serial) noexcept;
    std::uint32_t LastInputSerial() const noexcept;
    xdg_wm_base* XdgWmBase() const noexcept;
    bool HasXdgShell() const noexcept;

    static void HandleGlobal(void* data, wl_registry* registry, std::uint32_t name,
                             const char* interface, std::uint32_t version);
    static void HandleGlobalRemove(void* data, wl_registry* registry, std::uint32_t name);

private:
    LWindow* FindWindow(void* nativeWindowHandle) noexcept;
    const LWindow* FindWindow(void* nativeWindowHandle) const noexcept;

    wl_display* m_display = nullptr;
    wl_registry* m_registry = nullptr;
    wl_compositor* m_compositor = nullptr;
    wl_shm* m_shm = nullptr;
    wl_seat* m_seat = nullptr;
    wl_data_device_manager* m_dataDeviceManager = nullptr;
    xdg_wm_base* m_xdgWmBase = nullptr;
    bool m_hasXdgShell = false;
    LClipboard* m_clipboard = nullptr;
    std::uint32_t m_lastInputSerial = 0;
    std::vector<std::unique_ptr<LWindow>> m_windows;
    int m_exitCode = 0;
    bool m_consoleVisible = false;
};

}