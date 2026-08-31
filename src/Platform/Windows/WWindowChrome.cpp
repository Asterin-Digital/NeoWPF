#include <NeoWPF/Platform/Windows/WWindowChrome.h>

#include <windows.h>
#include <dwmapi.h>

#include <unordered_map>

namespace NeoWPF::Platform::Windows {
namespace {

bool IsHandleValid(void* nativeWindowHandle) noexcept
{
    return nativeWindowHandle != nullptr;
}

}

std::recursive_mutex& WWindowChrome::StateMutex()
{
    static std::recursive_mutex mutex;
    return mutex;
}

std::unordered_map<HWND, WWindowChrome::WindowState>& WWindowChrome::StateMap()
{
    static std::unordered_map<HWND, WindowState> map;
    return map;
}

NeoWPF::Contracts::ChromeMode WWindowChrome::DefaultChromeMode() const noexcept
{
    return NeoWPF::Contracts::ChromeMode::Native;
}

bool WWindowChrome::SupportsMode(NeoWPF::Contracts::ChromeMode mode) const noexcept
{
    return mode == NeoWPF::Contracts::ChromeMode::Native ||
           mode == NeoWPF::Contracts::ChromeMode::Custom;
}

bool WWindowChrome::SetChromeMode(void* nativeWindowHandle, NeoWPF::Contracts::ChromeMode mode) noexcept
{
    if (!IsHandleValid(nativeWindowHandle) || !SupportsMode(mode)) {
        return false;
    }

    const auto handle = static_cast<HWND>(nativeWindowHandle);
    std::lock_guard<std::recursive_mutex> lock(StateMutex());
    auto& state = StateMap()[handle];
    state.mode = mode;

    if (mode == NeoWPF::Contracts::ChromeMode::Custom) {
        LONG_PTR style = GetWindowLongPtrW(handle, GWL_STYLE);
        style &= ~WS_CAPTION;
        style &= ~WS_THICKFRAME;
        style &= ~WS_MAXIMIZEBOX;
        style &= ~WS_MINIMIZEBOX;
        SetWindowLongPtrW(handle, GWL_STYLE, style);

        LONG_PTR exStyle = GetWindowLongPtrW(handle, GWL_EXSTYLE);
        exStyle &= ~WS_EX_WINDOWEDGE;
        SetWindowLongPtrW(handle, GWL_EXSTYLE, exStyle);
        SetWindowPos(handle, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        return true;
    }

    LONG_PTR style = GetWindowLongPtrW(handle, GWL_STYLE);
    style |= WS_CAPTION;
    style |= WS_THICKFRAME;
    style |= WS_MAXIMIZEBOX;
    style |= WS_MINIMIZEBOX;
    SetWindowLongPtrW(handle, GWL_STYLE, style);

    LONG_PTR exStyle = GetWindowLongPtrW(handle, GWL_EXSTYLE);
    exStyle |= WS_EX_WINDOWEDGE;
    SetWindowLongPtrW(handle, GWL_EXSTYLE, exStyle);
    SetWindowPos(handle, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    return true;
}

NeoWPF::Contracts::ChromeMode WWindowChrome::GetChromeMode(void* nativeWindowHandle) const noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return NeoWPF::Contracts::ChromeMode::Native;
    }

    const auto handle = static_cast<HWND>(nativeWindowHandle);
    std::lock_guard<std::recursive_mutex> lock(StateMutex());
    const auto it = StateMap().find(handle);
    return it == StateMap().end() ? NeoWPF::Contracts::ChromeMode::Native : it->second.mode;
}

void WWindowChrome::SetTitleBarHitTest(void* nativeWindowHandle,
                                       NeoWPF::Contracts::ChromeHitTestCallback callback,
                                       void* userData) noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return;
    }

    const auto handle = static_cast<HWND>(nativeWindowHandle);
    std::lock_guard<std::recursive_mutex> lock(StateMutex());
    auto& state = StateMap()[handle];
    state.hitTestCallback = callback;
    state.hitTestUserData = userData;
}

NeoWPF::Contracts::ChromeHitTestCallback WWindowChrome::GetTitleBarHitTest(void* nativeWindowHandle,
                                                                           void** outUserData) const noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        if (outUserData) {
            *outUserData = nullptr;
        }
        return nullptr;
    }

    const auto handle = static_cast<HWND>(nativeWindowHandle);
    std::lock_guard<std::recursive_mutex> lock(StateMutex());
    const auto it = StateMap().find(handle);
    if (it == StateMap().end()) {
        if (outUserData) {
            *outUserData = nullptr;
        }
        return nullptr;
    }

    if (outUserData) {
        *outUserData = it->second.hitTestUserData;
    }
    return it->second.hitTestCallback;
}

void WWindowChrome::BeginInteractiveMove(void* nativeWindowHandle) noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return;
    }

    const auto handle = static_cast<HWND>(nativeWindowHandle);
    SendMessageW(handle, WM_SYSCOMMAND, SC_MOVE | 0x0002, 0);
}

void WWindowChrome::BeginInteractiveResize(void* nativeWindowHandle,
                                          NeoWPF::Contracts::ResizeEdge edge) noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return;
    }

    const auto handle = static_cast<HWND>(nativeWindowHandle);
    const auto side = static_cast<int>(edge);
    static constexpr int resizeMap[] = {
        WMSZ_LEFT,
        WMSZ_RIGHT,
        WMSZ_TOP,
        WMSZ_BOTTOM,
        WMSZ_TOPLEFT,
        WMSZ_TOPRIGHT,
        WMSZ_BOTTOMLEFT,
        WMSZ_BOTTOMRIGHT,
    };

    if (side >= 0 && side < static_cast<int>(std::size(resizeMap))) {
        SendMessageW(handle, WM_SYSCOMMAND, SC_SIZE | resizeMap[side], 0);
    }
}

void WWindowChrome::SetMaximized(void* nativeWindowHandle, bool maximized) noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return;
    }

    const auto handle = static_cast<HWND>(nativeWindowHandle);
    if (maximized) {
        ShowWindow(handle, SW_MAXIMIZE);
    } else {
        ShowWindow(handle, SW_RESTORE);
    }
}

bool WWindowChrome::IsZoomed(void* nativeWindowHandle) const noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return false;
    }

    return ::IsZoomed(static_cast<HWND>(nativeWindowHandle)) != FALSE;
}

bool WWindowChrome::IsMaximized(void* nativeWindowHandle) const noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return false;
    }

    return IsZoomed(nativeWindowHandle);
}

void WWindowChrome::SetMinimized(void* nativeWindowHandle) noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return;
    }

    ShowWindow(static_cast<HWND>(nativeWindowHandle), SW_MINIMIZE);
}

bool WWindowChrome::SetNativeCaptionColor(void* nativeWindowHandle, std::uint32_t argbColor) noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return false;
    }

    const auto handle = static_cast<HWND>(nativeWindowHandle);
    const auto color = static_cast<std::uint32_t>(argbColor);
    const auto result = DwmSetWindowAttribute(handle,
                                             DWMWA_CAPTION_COLOR,
                                             &color,
                                             sizeof(color));
    return result == S_OK;
}

}
