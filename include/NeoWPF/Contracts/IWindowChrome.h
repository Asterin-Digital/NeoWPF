/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>

namespace NeoWPF::Contracts {

enum class ChromeMode {
    Native,
    Custom,
    None,
};

enum class ResizeEdge {
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
};

enum class ChromeHitTestResult {
    Client,
    Drag,
    ResizeLeft,
    ResizeRight,
    ResizeTop,
    ResizeBottom,
    ResizeTopLeft,
    ResizeTopRight,
    ResizeBottomLeft,
    ResizeBottomRight,
    MinimizeButton,
    MaximizeButton,
    CloseButton,
};

using ChromeHitTestCallback = ChromeHitTestResult (*)(void* userData, int clientX, int clientY);

class IWindowChrome {
public:
    virtual ~IWindowChrome() = default;

    virtual ChromeMode DefaultChromeMode() const noexcept = 0;
    virtual bool SupportsMode(ChromeMode mode) const noexcept = 0;

    virtual bool SetChromeMode(void* nativeWindowHandle, ChromeMode mode) noexcept = 0;
    virtual ChromeMode GetChromeMode(void* nativeWindowHandle) const noexcept = 0;

    virtual void SetTitleBarHitTest(void* nativeWindowHandle,
                                   ChromeHitTestCallback callback,
                                   void* userData) noexcept = 0;

    virtual void BeginInteractiveMove(void* nativeWindowHandle) noexcept = 0;
    virtual void BeginInteractiveResize(void* nativeWindowHandle, ResizeEdge edge) noexcept = 0;
    virtual void SetMaximized(void* nativeWindowHandle, bool maximized) noexcept = 0;
    virtual bool IsZoomed(void* nativeWindowHandle) const noexcept = 0;
    virtual bool IsMaximized(void* nativeWindowHandle) const noexcept = 0;
    virtual void SetMinimized(void* nativeWindowHandle) noexcept = 0;

    virtual bool SetNativeCaptionColor(void* nativeWindowHandle,
                                      std::uint32_t argbColor) noexcept = 0;
};

}
