#include <NeoWPF/Platform/Windows/WWindowing.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <windows.h>

#ifdef CreateWindow
#undef CreateWindow
#endif
#ifdef CreateWindowEx
#undef CreateWindowEx
#endif
#ifdef CreateWindowExA
#undef CreateWindowExA
#endif
#ifdef CreateWindowExW
#undef CreateWindowExW
#endif

#include <cstdio>
#include <cmath>

namespace NeoWPF::Platform::Windows {
namespace {

bool EnsurePerMonitorDpiAwareness() noexcept
{
    static const bool isPerMonitorAware = [] {
        if (::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
            return true;
        }
        return ::GetAwarenessFromDpiAwarenessContext(::GetThreadDpiAwarenessContext()) ==
            DPI_AWARENESS_PER_MONITOR_AWARE;
    }();
    return isPerMonitorAware;
}

}

void* WWindowing::CreateWindow(
    const wchar_t* title,
    int width,
    int height,
    std::shared_ptr<NeoWPF::UI::Window> windowHost)
{
    auto& app = NeoWPF::Core::GetAppState();
    if (!app.hInst) {
        app.hInst = ::GetModuleHandleW(nullptr);
    }

    if (!EnsurePerMonitorDpiAwareness()) {
        NeoWPF::Diagnostics::Catastrophic(
            "CreateWindow requires per-monitor DPI awareness");
        return nullptr;
    }

    const bool transparent = windowHost && windowHost->IsTransparent();
    void* nativeHandle = NeoWPF::Platform::Window::CreateWindow(app.hInst, title, width, height, transparent);
    if (!nativeHandle) {
        NeoWPF::Diagnostics::Catastrophic("CreateWindow failed for title=%ls width=%d height=%d", title, width, height);
        return nullptr;
    }

    NeoWPF::Core::WindowSession windowSession;
    windowSession.renderSurface.nativeWindowHandle = nativeHandle;
    windowSession.width = width;
    windowSession.height = height;
    windowSession.needsLayout = true;
    windowSession.windowHost = windowHost ? std::move(windowHost) : std::make_shared<NeoWPF::UI::Window>();
    windowSession.contentRoot = nullptr;

    {
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        windowSession.lifecycle.Activate();
        app.windowSessions.push_back(windowSession);
        app.hasCreatedWindow = true;
    }

    if (windowSession.windowHost->GetTitleBar()) {
        if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
            if (auto* chrome = platform->Chrome()) {
                chrome->SetChromeMode(nativeHandle, NeoWPF::Contracts::ChromeMode::Custom);
            }
        }
    }

    const auto hwnd = static_cast<HWND>(nativeHandle);
    ::ShowWindow(hwnd, SW_SHOWNORMAL);
    ::UpdateWindow(hwnd);
    ::RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
    return nativeHandle;
}

bool WWindowing::GetNativeWindowInfo(void* nativeWindowHandle, NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept
{
    outInfo = {};
    if (!IsWindowAlive(nativeWindowHandle)) {
        return false;
    }

    outInfo.system = NeoWPF::Contracts::NativeWindowSystem::Win32;
    outInfo.surfaceAlpha = ::GetPropW(static_cast<HWND>(nativeWindowHandle), L"NeoWPF.Transparent")
        ? NeoWPF::Contracts::SurfaceAlpha::Premultiplied
        : NeoWPF::Contracts::SurfaceAlpha::Opaque;
    outInfo.window = nativeWindowHandle;
    return true;
}

bool WWindowing::IsWindowAlive(void* nativeWindowHandle) const noexcept
{
    return nativeWindowHandle && ::IsWindow(static_cast<HWND>(nativeWindowHandle)) != FALSE;
}

void WWindowing::GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth, unsigned int& outHeight) const noexcept
{
    outWidth = 0;
    outHeight = 0;
    if (!IsWindowAlive(nativeWindowHandle)) {
        return;
    }

    RECT clientRect{};
    if (::GetClientRect(static_cast<HWND>(nativeWindowHandle), &clientRect)) {
        outWidth = static_cast<unsigned int>(clientRect.right - clientRect.left);
        outHeight = static_cast<unsigned int>(clientRect.bottom - clientRect.top);
    }
}

bool WWindowing::GetWindowScaleFactor(void* nativeWindowHandle,
                                      double& outScaleFactor) const noexcept
{
    outScaleFactor = 0.0;
    if (!IsWindowAlive(nativeWindowHandle)) {
        return false;
    }

    const UINT dpi = ::GetDpiForWindow(static_cast<HWND>(nativeWindowHandle));
    if (dpi == 0) {
        return false;
    }

    outScaleFactor = static_cast<double>(dpi) / 96.0;
    return std::isfinite(outScaleFactor) && outScaleFactor > 0.0;
}

bool WWindowing::GetWindowInsets(void* nativeWindowHandle,
                                 NeoWPF::Contracts::WindowInsets& outSafeArea,
                                 NeoWPF::Contracts::WindowInsets& outKeyboard) const noexcept
{
    if (!IsWindowAlive(nativeWindowHandle)) {
        return false;
    }
    outSafeArea = {};
    outKeyboard = {};
    return true;
}

bool WWindowing::GetInputProfile(void* nativeWindowHandle,
                                 NeoWPF::Contracts::InputProfile& outProfile) const noexcept
{
    if (!IsWindowAlive(nativeWindowHandle)) {
        return false;
    }
    outProfile.primaryPointer = NeoWPF::Contracts::PointerPrecision::Fine;
    outProfile.canHover = true;
    outProfile.hasTouch = ::GetSystemMetrics(SM_MAXIMUMTOUCHES) > 0;
    if (outProfile.hasTouch) {
        outProfile.primaryPointer = NeoWPF::Contracts::PointerPrecision::Coarse;
        outProfile.canHover = false;
    }
    return true;
}

void WWindowing::ResizeWindow(void* nativeWindowHandle, int width, int height) noexcept
{
    if (IsWindowAlive(nativeWindowHandle)) {
        ::SetWindowPos(static_cast<HWND>(nativeWindowHandle), nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void WWindowing::ShowWindow(void* nativeWindowHandle) noexcept
{
    if (IsWindowAlive(nativeWindowHandle)) {
        const auto hwnd = static_cast<HWND>(nativeWindowHandle);
        ::ShowWindow(hwnd, SW_SHOWNORMAL);
        ::UpdateWindow(hwnd);
    }
}

void WWindowing::HideWindow(void* nativeWindowHandle) noexcept
{
    if (IsWindowAlive(nativeWindowHandle)) {
        ::ShowWindow(static_cast<HWND>(nativeWindowHandle), SW_HIDE);
    }
}

void WWindowing::CloseWindow(void* nativeWindowHandle) noexcept
{
    if (nativeWindowHandle) {
        const auto hwnd = static_cast<HWND>(nativeWindowHandle);
        if (NeoWPF::Core::CloseWindow(nativeWindowHandle) && IsWindow(hwnd)) {
            ::DestroyWindow(hwnd);
        }
    }
}

void WWindowing::InvalidateWindow(void* nativeWindowHandle) noexcept
{
    if (IsWindowAlive(nativeWindowHandle)) {
        const auto hwnd = static_cast<HWND>(nativeWindowHandle);
        ::UpdateWindow(hwnd);
        ::RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

bool WWindowing::PumpMessages(std::chrono::milliseconds timeout)
{
    const auto waitMilliseconds = static_cast<DWORD>(timeout.count());
    const DWORD waitResult = ::MsgWaitForMultipleObjects(0, nullptr, FALSE, waitMilliseconds, QS_ALLINPUT);
    if (waitResult == WAIT_FAILED) {
        return false;
    }

    MSG message{};
    while (::PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            m_exitCode = static_cast<int>(message.wParam);
            return false;
        }
        ::TranslateMessage(&message);
        ::DispatchMessage(&message);
    }
    return true;
}

int WWindowing::ExitCode() const noexcept
{
    return m_exitCode;
}

void WWindowing::RequestQuit(int exitCode) noexcept
{
    m_exitCode = exitCode;
    ::PostQuitMessage(exitCode);
}

void WWindowing::ShowFatalError(const std::wstring& title, const std::wstring& message) noexcept
{
    ::MessageBoxW(nullptr, message.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
}

void WWindowing::SetConsoleVisible(bool visible) noexcept
{
    auto& appState = NeoWPF::Core::GetAppState();
    if (appState.consoleVisible.load() == visible) {
        return;
    }

    appState.consoleVisible.store(visible);
    if (visible) {
        if (::AllocConsole()) {
            FILE* output = nullptr;
            freopen_s(&output, "CONOUT$", "w", stdout);
            freopen_s(&output, "CONOUT$", "w", stderr);
        }
    } else {
        ::FreeConsole();
    }
}

bool WWindowing::IsConsoleVisible() const noexcept
{
    return NeoWPF::Core::GetAppState().consoleVisible.load();
}

}
