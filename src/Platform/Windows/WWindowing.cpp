#include <NeoWPF/Platform/Windows/WWindowing.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/NativeWindow.h>
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

namespace NeoWPF::Platform::Windows {

void* WWindowing::CreateWindow(const wchar_t* title, int width, int height)
{
    auto& app = NeoWPF::Core::GetAppState();
    if (!app.hInst) {
        app.hInst = ::GetModuleHandleW(nullptr);
    }

    void* nativeHandle = NeoWPF::Platform::Window::CreateWindow(app.hInst, title, width, height);
    if (!nativeHandle) {
        NeoWPF::Diagnostics::LogError("CreateWindow failed for title=%ls width=%d height=%d", title, width, height);
        return nullptr;
    }

    NeoWPF::Core::WindowState windowState;
    windowState.nativeWindowHandle = nativeHandle;
    windowState.width = width;
    windowState.height = height;
    windowState.needsLayout = true;
    windowState.isPrimaryWindow = app.windows.empty();
    windowState.windowHost = std::make_shared<NeoWPF::UI::Window>();
    windowState.contentRoot = nullptr;

    {
        std::lock_guard<std::mutex> lock(app.windowsMutex);
        app.windows.push_back(windowState);
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
        if (NeoWPF::Core::CloseApplicationWindow(nativeWindowHandle) && IsWindow(hwnd)) {
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
