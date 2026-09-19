#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Platform/Windows/WWindowChrome.h>
#include <NeoWPF/UI/Controls/DefaultTitleBar.h>
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

// Undef Windows Macros
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
//

namespace {

const wchar_t* NeoWPFWindowClassName = L"NeoWPFWindowClass";

LRESULT CALLBACK NeoWPFWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        return 0;

    case WM_NCCALCSIZE:
    {
        if (wParam == TRUE) {
            auto* platform = NeoWPF::Platform::CurrentPlatform();
            if (platform && platform->Chrome()) {
                const auto mode = platform->Chrome()->GetChromeMode(hwnd);
                if (mode == NeoWPF::Contracts::ChromeMode::Custom) {
                    if (lParam) {
                        auto* calc = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                        calc->rgrc[0].top += 0;
                    }
                    return 0;
                }
            }
        }
        break;
    }

    case WM_NCHITTEST:
    {
        auto* platform = NeoWPF::Platform::CurrentPlatform();
        if (platform && platform->Chrome()) {
            const auto mode = platform->Chrome()->GetChromeMode(hwnd);
            if (mode == NeoWPF::Contracts::ChromeMode::Custom) {
                void* userData = nullptr;
                const auto callback = static_cast<NeoWPF::Platform::Windows::WWindowChrome*>(platform->Chrome())->GetTitleBarHitTest(hwnd, &userData);
                if (callback) {
                    POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                    ScreenToClient(hwnd, &pt);
                    const auto result = callback(userData, pt.x, pt.y);
                    switch (result) {
                    case NeoWPF::Contracts::ChromeHitTestResult::Drag:
                        return HTCAPTION;
                    case NeoWPF::Contracts::ChromeHitTestResult::ResizeLeft:
                        return HTLEFT;
                    case NeoWPF::Contracts::ChromeHitTestResult::ResizeRight:
                        return HTRIGHT;
                    case NeoWPF::Contracts::ChromeHitTestResult::ResizeTop:
                        return HTTOP;
                    case NeoWPF::Contracts::ChromeHitTestResult::ResizeBottom:
                        return HTBOTTOM;
                    case NeoWPF::Contracts::ChromeHitTestResult::ResizeTopLeft:
                        return HTTOPLEFT;
                    case NeoWPF::Contracts::ChromeHitTestResult::ResizeTopRight:
                        return HTTOPRIGHT;
                    case NeoWPF::Contracts::ChromeHitTestResult::ResizeBottomLeft:
                        return HTBOTTOMLEFT;
                    case NeoWPF::Contracts::ChromeHitTestResult::ResizeBottomRight:
                        return HTBOTTOMRIGHT;
                    case NeoWPF::Contracts::ChromeHitTestResult::MinimizeButton:
                        return HTMINBUTTON;
                    case NeoWPF::Contracts::ChromeHitTestResult::MaximizeButton:
                        return HTMAXBUTTON;
                    case NeoWPF::Contracts::ChromeHitTestResult::CloseButton:
                        return HTCLOSE;
                    case NeoWPF::Contracts::ChromeHitTestResult::Client:
                    default:
                        return HTCLIENT;
                    }
                }
            }
        }
        break;
    }

    case WM_DESTROY:
        return 0;

    case WM_CLOSE:
    {
        auto* platform = NeoWPF::Platform::CurrentPlatform();
        if (platform) {
            platform->Windowing().CloseWindow(hwnd);
        } else {
            NeoWPF::Core::CloseApplicationWindow(hwnd);
            if (IsWindow(hwnd)) {
                ::DestroyWindow(hwnd);
            }
        }
        return 0;
    }

    case WM_ERASEBKGND:
        return TRUE;

    case WM_MOUSEMOVE:
    {
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        std::uint32_t buttonState = static_cast<std::uint32_t>(wParam);
        NeoWPF::Input::InputManager::GetInstance().ProcessMouseInput(x, y, buttonState);
        return 0;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    {
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        std::uint32_t buttonState = static_cast<std::uint32_t>(wParam);
        if (uMsg == WM_LBUTTONDOWN)
            buttonState |= 0x01;
        else
            buttonState &= ~0x01;
        NeoWPF::Input::InputManager::GetInstance().ProcessMouseInput(x, y, buttonState);
        return 0;
    }

    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    {
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        std::uint32_t buttonState = static_cast<std::uint32_t>(wParam);
        if (uMsg == WM_RBUTTONDOWN)
            buttonState |= 0x02;
        else
            buttonState &= ~0x02;
        NeoWPF::Input::InputManager::GetInstance().ProcessMouseInput(x, y, buttonState);
        return 0;
    }

    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    {
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        std::uint32_t buttonState = static_cast<std::uint32_t>(wParam);
        if (uMsg == WM_MBUTTONDOWN)
            buttonState |= 0x04;
        else
            buttonState &= ~0x04;
        NeoWPF::Input::InputManager::GetInstance().ProcessMouseInput(x, y, buttonState);
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        POINT screenPoint{ x, y };
        ScreenToClient(hwnd, &screenPoint);
        x = screenPoint.x;
        y = screenPoint.y;
        int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        std::uint32_t buttonState = GET_KEYSTATE_WPARAM(wParam);
        NeoWPF::Input::InputManager::GetInstance().ProcessMouseInput(x, y, buttonState, wheelDelta);
        return 0;
    }

    case WM_KEYDOWN:
    {
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        int keyCode = static_cast<int>(wParam);
        int repeatCount = static_cast<int>(lParam) & 0xFFFF;
        NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(keyCode, true, 0, repeatCount);
        return 0;
    }

    case WM_KEYUP:
    {
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        int keyCode = static_cast<int>(wParam);
        int repeatCount = static_cast<int>(lParam) & 0xFFFF;
        NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(keyCode, false, 0, repeatCount);
        return 0;
    }

    case WM_CHAR:
    {
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        wchar_t charCode = static_cast<wchar_t>(wParam);
        NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(0, true, charCode, 1);
        return 0;
    }

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void EnsureNeoWPFWindowClass(HINSTANCE hInst)
{
    WNDCLASSW existing = {};
    if (GetClassInfoW(nullptr, NeoWPFWindowClassName, &existing)) {
        return;
    }

    WNDCLASSW wc = {};
    wc.lpfnWndProc = NeoWPFWindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = NeoWPFWindowClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);
}

}

namespace NeoWPF::Platform {

void Window::Resize(int width, int height) const noexcept {
    HWND hwnd = static_cast<HWND>(m_nativeHandle);
    if (hwnd) {
        SetWindowPos(hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Window::Show() const noexcept {
    HWND hwnd = static_cast<HWND>(m_nativeHandle);
    if (hwnd) {
        ::ShowWindow(hwnd, SW_SHOWNORMAL);
        ::UpdateWindow(hwnd);
    }
}

void Window::Hide() const noexcept {
    HWND hwnd = static_cast<HWND>(m_nativeHandle);
    if (hwnd) {
        ::ShowWindow(hwnd, SW_HIDE);
    }
}

void Window::Close() const noexcept {
    HWND hwnd = static_cast<HWND>(m_nativeHandle);
    if (hwnd) {
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
    }
}

void Window::Minimize() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        if (auto* chrome = platform->Chrome()) {
            chrome->SetMinimized(m_nativeHandle);
        }
    }
}

void Window::ToggleMaximize() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        if (auto* chrome = platform->Chrome()) {
            const bool maximized = chrome->IsMaximized(m_nativeHandle);
            chrome->SetMaximized(m_nativeHandle, !maximized);
        }
    }
}

void Window::SetChromeMode(NeoWPF::Contracts::ChromeMode mode) const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        if (auto* chrome = platform->Chrome()) {
            chrome->SetChromeMode(m_nativeHandle, mode);
        }
    }
}

NeoWPF::Contracts::ChromeMode Window::GetChromeMode() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        if (auto* chrome = platform->Chrome()) {
            return chrome->GetChromeMode(m_nativeHandle);
        }
    }
    return NeoWPF::Contracts::ChromeMode::Native;
}

void Window::SetContent(std::shared_ptr<NeoWPF::UI::Visual> content) noexcept
{
    HWND hwnd = static_cast<HWND>(m_nativeHandle);
    if (hwnd) {
        SetWindowContent(hwnd, std::move(content));
        return;
    }

    m_content = std::move(content);
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetContent() const noexcept
{
    HWND hwnd = static_cast<HWND>(m_nativeHandle);
    if (hwnd) {
        return GetWindowContent(hwnd);
    }

    return m_content;
}

void Window::SetTitleBar(std::shared_ptr<NeoWPF::UI::Visual> titleBar) noexcept
{
    HWND hwnd = static_cast<HWND>(m_nativeHandle);
    if (hwnd) {
        SetWindowTitleBar(hwnd, std::move(titleBar));
        return;
    }

    m_titleBar = std::move(titleBar);
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetTitleBar() const noexcept
{
    HWND hwnd = static_cast<HWND>(m_nativeHandle);
    if (hwnd) {
        return GetWindowTitleBar(hwnd);
    }

    return m_titleBar;
}

void Window::SetWindowContent(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> content)
{
    HWND hwnd = static_cast<HWND>(nativeHandle);
    if (!hwnd) {
        return;
    }

    auto& windows = NeoWPF::Core::GetAppState().windows;
    for (auto& windowState : windows) {
        if (windowState.nativeWindowHandle == hwnd) {
            windowState.contentRoot = content.get();
            windowState.windowHost->SetContent(content);
            return;
        }
    }
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetWindowContent(void* nativeHandle)
{
    HWND hwnd = static_cast<HWND>(nativeHandle);
    if (!hwnd) {
        return nullptr;
    }

    const auto& windows = NeoWPF::Core::GetAppState().windows;
    for (const auto& windowState : windows) {
        if (windowState.nativeWindowHandle == hwnd) {
            return windowState.windowHost->GetContent();
        }
    }
    return nullptr;
}

void Window::SetWindowTitleBar(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> titleBar)
{
    HWND hwnd = static_cast<HWND>(nativeHandle);
    if (!hwnd) {
        return;
    }

    const bool hasTitleBar = static_cast<bool>(titleBar);

    auto* platform = NeoWPF::Platform::CurrentPlatform();
    auto* chrome = platform ? platform->Chrome() : nullptr;

    auto& windows = NeoWPF::Core::GetAppState().windows;
    for (auto& windowState : windows) {
        if (windowState.nativeWindowHandle == hwnd) {
            windowState.windowHost->SetTitleBar(titleBar);

            if (chrome) {
                chrome->SetTitleBarHitTest(hwnd, nullptr, nullptr);
            }

            if (hasTitleBar && chrome) {
                chrome->SetChromeMode(hwnd, NeoWPF::Contracts::ChromeMode::Custom);
            }
            return;
        }
    }
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetWindowTitleBar(void* nativeHandle)
{
    HWND hwnd = static_cast<HWND>(nativeHandle);
    if (!hwnd) {
        return nullptr;
    }

    const auto& windows = NeoWPF::Core::GetAppState().windows;
    for (const auto& windowState : windows) {
        if (windowState.nativeWindowHandle == hwnd) {
            return windowState.windowHost->GetTitleBar();
        }
    }
    return nullptr;
}

void* Window::CreateWindow(void* hInst, const wchar_t* title, int w, int h)
{
    HINSTANCE hInstance = static_cast<HINSTANCE>(hInst);
    EnsureNeoWPFWindowClass(hInstance);
    HWND hwnd = CreateWindowExW(0, NeoWPFWindowClassName, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, nullptr, nullptr, hInstance, nullptr);
    return hwnd;
}

}
