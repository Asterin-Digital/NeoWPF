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

#include <NeoWPF/Contracts/IWindowChrome.h>

#include <X11/Xlib.h>

#undef Success
#undef None

#include <mutex>
#include <unordered_map>

namespace NeoWPF::Platform::Linux {

class XWindowing;

class XWindowChrome final : public NeoWPF::Contracts::IWindowChrome {
public:
    explicit XWindowChrome(XWindowing& windowing) noexcept;
    ~XWindowChrome() override = default;

    NeoWPF::Contracts::ChromeMode DefaultChromeMode() const noexcept override;
    bool SupportsMode(NeoWPF::Contracts::ChromeMode mode) const noexcept override;
    bool SetChromeMode(void* nativeWindowHandle, NeoWPF::Contracts::ChromeMode mode) noexcept override;
    NeoWPF::Contracts::ChromeMode GetChromeMode(void* nativeWindowHandle) const noexcept override;
    void SetTitleBarHitTest(void* nativeWindowHandle,
                            NeoWPF::Contracts::ChromeHitTestCallback callback,
                            void* userData) noexcept override;
    void BeginInteractiveMove(void* nativeWindowHandle) noexcept override;
    void BeginInteractiveResize(void* nativeWindowHandle, NeoWPF::Contracts::ResizeEdge edge) noexcept override;
    void SetMaximized(void* nativeWindowHandle, bool maximized) noexcept override;
    bool IsZoomed(void* nativeWindowHandle) const noexcept override;
    bool IsMaximized(void* nativeWindowHandle) const noexcept override;
    void SetMinimized(void* nativeWindowHandle) noexcept override;
    bool SetNativeCaptionColor(void* nativeWindowHandle, std::uint32_t argbColor) noexcept override;

private:
    XWindowing& m_windowing;
    mutable std::recursive_mutex m_stateMutex;
    std::unordered_map<void*, NeoWPF::Contracts::ChromeMode> m_modes;
};

}
