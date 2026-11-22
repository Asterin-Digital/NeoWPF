#include <NeoWPF/Platform/Linux/Wayland/WlWindowChrome.h>

#include <NeoWPF/Platform/Linux/LClientSideTitleBar.h>
#include <NeoWPF/Platform/Linux/Wayland/WlWindow.h>
#include <NeoWPF/Platform/Linux/Wayland/WlWindowing.h>

#include <unordered_map>

namespace NeoWPF::Platform::Linux {
namespace {

bool IsHandleValid(void* nativeWindowHandle) noexcept
{
    return nativeWindowHandle != nullptr;
}

}

std::recursive_mutex& WlWindowChrome::StateMutex()
{
    static std::recursive_mutex mutex;
    return mutex;
}

std::unordered_map<void*, WlWindowChrome::WindowState>& WlWindowChrome::StateMap()
{
    static std::unordered_map<void*, WindowState> map;
    return map;
}

NeoWPF::Contracts::ChromeMode WlWindowChrome::DefaultChromeMode() const noexcept
{
    return NeoWPF::Contracts::ChromeMode::Native;
}

bool WlWindowChrome::SupportsMode(NeoWPF::Contracts::ChromeMode mode) const noexcept
{
    return mode == NeoWPF::Contracts::ChromeMode::Native ||
           mode == NeoWPF::Contracts::ChromeMode::Custom ||
           mode == NeoWPF::Contracts::ChromeMode::None;
}

bool WlWindowChrome::SetChromeMode(void* nativeWindowHandle, NeoWPF::Contracts::ChromeMode mode) noexcept
{
    if (!IsHandleValid(nativeWindowHandle) || !SupportsMode(mode)) {
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(StateMutex());
    auto& state = StateMap()[nativeWindowHandle];
    state.mode = mode;

    if (mode == NeoWPF::Contracts::ChromeMode::None) {
        EnsureClientTitleBar(nativeWindowHandle, NeoWPF::Contracts::ChromeMode::None);
    } else {
        EnsureClientTitleBar(nativeWindowHandle, mode);
    }

    return true;
}

NeoWPF::Contracts::ChromeMode WlWindowChrome::GetChromeMode(void* nativeWindowHandle) const noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return DefaultChromeMode();
    }

    std::lock_guard<std::recursive_mutex> lock(StateMutex());
    const auto it = StateMap().find(nativeWindowHandle);
    return it == StateMap().end() ? DefaultChromeMode() : it->second.mode;
}

void WlWindowChrome::SetTitleBarHitTest(void* nativeWindowHandle,
                                       NeoWPF::Contracts::ChromeHitTestCallback callback,
                                       void* userData) noexcept
{
    if (!IsHandleValid(nativeWindowHandle)) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(StateMutex());
    auto& state = StateMap()[nativeWindowHandle];
    state.hitTestCallback = callback;
    state.hitTestUserData = userData;
}

void WlWindowChrome::BeginInteractiveMove(void* nativeWindowHandle) noexcept
{
    auto* window = static_cast<WlWindow*>(nativeWindowHandle);
    if (!window) {
        return;
    }

    if (window->IsValid() && window->Display()) {
        window->Minimize();
    }
}

void WlWindowChrome::BeginInteractiveResize(void* nativeWindowHandle,
                                          NeoWPF::Contracts::ResizeEdge edge) noexcept
{
    auto* window = static_cast<WlWindow*>(nativeWindowHandle);
    if (!window) {
        return;
    }

    (void)edge;
    if (window->IsValid()) {
        window->Maximize(true);
    }
}

void WlWindowChrome::SetMaximized(void* nativeWindowHandle, bool maximized) noexcept
{
    auto* window = static_cast<WlWindow*>(nativeWindowHandle);
    if (!window) {
        return;
    }

    window->Maximize(maximized);
}

bool WlWindowChrome::IsZoomed(void* nativeWindowHandle) const noexcept
{
    auto* window = static_cast<WlWindow*>(nativeWindowHandle);
    return window && window->IsMaximized();
}

bool WlWindowChrome::IsMaximized(void* nativeWindowHandle) const noexcept
{
    auto* window = static_cast<WlWindow*>(nativeWindowHandle);
    return window && window->IsMaximized();
}

void WlWindowChrome::SetMinimized(void* nativeWindowHandle) noexcept
{
    auto* window = static_cast<WlWindow*>(nativeWindowHandle);
    if (!window) {
        return;
    }

    window->Minimize();
}

bool WlWindowChrome::SetNativeCaptionColor(void* nativeWindowHandle, std::uint32_t argbColor) noexcept
{
    (void)nativeWindowHandle;
    (void)argbColor;
    return false;
}

}
