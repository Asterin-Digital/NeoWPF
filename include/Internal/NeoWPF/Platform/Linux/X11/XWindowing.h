/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowing.h>

#include <X11/Xlib.h>

#undef Success
#undef None

#include <memory>
#include <vector>

namespace NeoWPF::Platform::Linux {

class XClipboard;
class XWindow;

class XWindowing final : public NeoWPF::Contracts::IWindowing {
public:
    XWindowing();
    ~XWindowing() override;

    XWindowing(const XWindowing&) = delete;
    XWindowing& operator=(const XWindowing&) = delete;

    bool Connect();
    void Disconnect() noexcept;
    bool IsConnected() const noexcept;
    Display* DisplayHandle() const noexcept;
    ::Window RootHandle() const noexcept;
    Atom WmProtocolsAtom() const noexcept;
    Atom WmDeleteWindowAtom() const noexcept;

    void* CreateWindow(
        const wchar_t* title,
        int width,
        int height,
        std::shared_ptr<NeoWPF::UI::Window> windowHost = nullptr) override;
    bool GetNativeWindowInfo(void* nativeWindowHandle, NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept override;
    bool IsWindowAlive(void* nativeWindowHandle) const noexcept override;
    void GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth, unsigned int& outHeight) const noexcept override;
    bool GetWindowScaleFactor(void* nativeWindowHandle, double& outScaleFactor) const noexcept override;
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

    void AttachClipboard(XClipboard* clipboard) noexcept;
    void DetachClipboard(XClipboard* clipboard) noexcept;
    XWindow* FindWindow(void* nativeWindowHandle) noexcept;
    const XWindow* FindWindow(void* nativeWindowHandle) const noexcept;

private:
    Display* m_display = nullptr;
    ::Window m_root = 0;
    int m_screen = 0;
    Atom m_wmProtocols = 0;
    Atom m_wmDeleteWindow = 0;
    XClipboard* m_clipboard = nullptr;
    std::vector<std::unique_ptr<XWindow>> m_windows;
    int m_exitCode = 0;
    bool m_consoleVisible = false;
};

}
