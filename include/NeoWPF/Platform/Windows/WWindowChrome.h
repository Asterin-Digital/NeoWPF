/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowChrome.h>

#include <windows.h>

#include <mutex>
#include <unordered_map>

namespace NeoWPF::Platform::Windows {

class WWindowChrome final : public NeoWPF::Contracts::IWindowChrome {
public:
    struct WindowState {
        NeoWPF::Contracts::ChromeMode mode = NeoWPF::Contracts::ChromeMode::Native;
        NeoWPF::Contracts::ChromeHitTestCallback hitTestCallback = nullptr;
        void* hitTestUserData = nullptr;
    };

    WWindowChrome() = default;
    ~WWindowChrome() override = default;

    NeoWPF::Contracts::ChromeMode DefaultChromeMode() const noexcept override;
    bool SupportsMode(NeoWPF::Contracts::ChromeMode mode) const noexcept override;

    bool SetChromeMode(void* nativeWindowHandle, NeoWPF::Contracts::ChromeMode mode) noexcept override;
    NeoWPF::Contracts::ChromeMode GetChromeMode(void* nativeWindowHandle) const noexcept override;

    void SetTitleBarHitTest(void* nativeWindowHandle,
                            NeoWPF::Contracts::ChromeHitTestCallback callback,
                            void* userData) noexcept override;

    NeoWPF::Contracts::ChromeHitTestCallback GetTitleBarHitTest(void* nativeWindowHandle,
                                                                void** outUserData) const noexcept;

    void BeginInteractiveMove(void* nativeWindowHandle) noexcept override;
    void BeginInteractiveResize(void* nativeWindowHandle, NeoWPF::Contracts::ResizeEdge edge) noexcept override;
    void SetMaximized(void* nativeWindowHandle, bool maximized) noexcept override;
    bool IsZoomed(void* nativeWindowHandle) const noexcept override;
    bool IsMaximized(void* nativeWindowHandle) const noexcept override;
    void SetMinimized(void* nativeWindowHandle) noexcept override;

    bool SetNativeCaptionColor(void* nativeWindowHandle, std::uint32_t argbColor) noexcept override;

private:
    static std::recursive_mutex& StateMutex();
    static std::unordered_map<HWND, WindowState>& StateMap();
};

}
