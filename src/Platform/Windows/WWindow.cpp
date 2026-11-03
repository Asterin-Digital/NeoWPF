#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Platform/Windows/WWindowing.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Keyboard.h>
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

int TranslateWindowsVirtualKey(WPARAM virtualKey) noexcept
{
    using NeoWPF::Input::Key;

    if (virtualKey >= '0' && virtualKey <= '9') {
        return static_cast<int>(Key::D0) + static_cast<int>(virtualKey - '0');
    }
    if (virtualKey >= 'A' && virtualKey <= 'Z') {
        return static_cast<int>(Key::A) + static_cast<int>(virtualKey - 'A');
    }
    if (virtualKey >= VK_F1 && virtualKey <= VK_F24) {
        return static_cast<int>(Key::F1) + static_cast<int>(virtualKey - VK_F1);
    }
    if (virtualKey >= VK_NUMPAD0 && virtualKey <= VK_NUMPAD9) {
        return static_cast<int>(Key::NumPad0) + static_cast<int>(virtualKey - VK_NUMPAD0);
    }

    switch (virtualKey) {
    case VK_BACK: return static_cast<int>(Key::Back);
    case VK_TAB: return static_cast<int>(Key::Tab);
    case VK_RETURN: return static_cast<int>(Key::Enter);
    case VK_PAUSE: return static_cast<int>(Key::Pause);
    case VK_CAPITAL: return static_cast<int>(Key::CapsLock);
    case VK_ESCAPE: return static_cast<int>(Key::Escape);
    case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: return static_cast<int>(Key::Shift);
    case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: return static_cast<int>(Key::Control);
    case VK_MENU: case VK_LMENU: case VK_RMENU: return static_cast<int>(Key::Alt);
    case VK_PRIOR: return static_cast<int>(Key::PageUp);
    case VK_NEXT: return static_cast<int>(Key::PageDown);
    case VK_END: return static_cast<int>(Key::End);
    case VK_HOME: return static_cast<int>(Key::Home);
    case VK_LEFT: return static_cast<int>(Key::Left);
    case VK_UP: return static_cast<int>(Key::Up);
    case VK_RIGHT: return static_cast<int>(Key::Right);
    case VK_DOWN: return static_cast<int>(Key::Down);
    case VK_INSERT: return static_cast<int>(Key::Insert);
    case VK_DELETE: return static_cast<int>(Key::Delete);
    case VK_MULTIPLY: return static_cast<int>(Key::Multiply);
    case VK_ADD: return static_cast<int>(Key::Add);
    case VK_SEPARATOR: return static_cast<int>(Key::Separator);
    case VK_SUBTRACT: return static_cast<int>(Key::Subtract);
    case VK_DECIMAL: return static_cast<int>(Key::Decimal);
    case VK_DIVIDE: return static_cast<int>(Key::Divide);
    case VK_SPACE: return static_cast<int>(Key::Space);
    case VK_OEM_1: return static_cast<int>(Key::Oem1);
    case VK_OEM_PLUS: return static_cast<int>(Key::OemPlus);
    case VK_OEM_COMMA: return static_cast<int>(Key::OemComma);
    case VK_OEM_MINUS: return static_cast<int>(Key::OemMinus);
    case VK_OEM_PERIOD: return static_cast<int>(Key::OemPeriod);
    case VK_OEM_2: return static_cast<int>(Key::Oem2);
    case VK_OEM_3: return static_cast<int>(Key::Oem3);
    case VK_OEM_4: return static_cast<int>(Key::Oem4);
    case VK_OEM_5: return static_cast<int>(Key::Oem5);
    case VK_OEM_6: return static_cast<int>(Key::Oem6);
    case VK_OEM_7: return static_cast<int>(Key::Oem7);
    case VK_OEM_8: return static_cast<int>(Key::Oem8);
    case VK_OEM_102: return static_cast<int>(Key::Oem102);
    case VK_VOLUME_MUTE: return static_cast<int>(Key::VolumeMute);
    case VK_VOLUME_DOWN: return static_cast<int>(Key::VolumeDown);
    case VK_VOLUME_UP: return static_cast<int>(Key::VolumeUp);
    case VK_MEDIA_NEXT_TRACK: return static_cast<int>(Key::MediaNextTrack);
    case VK_MEDIA_PREV_TRACK: return static_cast<int>(Key::MediaPreviousTrack);
    case VK_MEDIA_STOP: return static_cast<int>(Key::MediaStop);
    case VK_MEDIA_PLAY_PAUSE: return static_cast<int>(Key::MediaPlayPause);
    default: return static_cast<int>(Key::None);
    }
}

void SubmitNativePointerInput(NeoWPF::Input::Core::PointerPhase phase,
                              int x, int y, std::uint32_t buttons,
                              std::uint32_t changedButton = 0, double wheelDelta = 0.0)
{
    auto& input = static_cast<NeoWPF::Contracts::IPointerInput&>(
        NeoWPF::Input::InputManager::GetInstance());
    input.SubmitPointerEvent(NeoWPF::Input::Core::PointerEventArgs(
        0,
        NeoWPF::Input::Core::PointerDeviceType::Mouse, phase,
        x, y, buttons, changedButton, 0.0f, wheelDelta));
}

LRESULT CALLBACK NeoWPFWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        NeoWPF::Core::SetInputRoot(hwnd);
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
            NeoWPF::Core::CloseWindow(hwnd);
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
        NeoWPF::Core::SetInputRoot(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        std::uint32_t buttonState = static_cast<std::uint32_t>(wParam);
        SubmitNativePointerInput(NeoWPF::Input::Core::PointerPhase::Move,
                     x, y, buttonState);
        return 0;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    {
        NeoWPF::Core::SetInputRoot(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        std::uint32_t buttonState = static_cast<std::uint32_t>(wParam);
        if (uMsg == WM_LBUTTONDOWN)
            buttonState |= 0x01;
        else
            buttonState &= ~0x01;
        SubmitNativePointerInput(
            uMsg == WM_LBUTTONDOWN ? NeoWPF::Input::Core::PointerPhase::Down : NeoWPF::Input::Core::PointerPhase::Up,
            x, y, buttonState, 0x01);
        return 0;
    }

    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    {
        NeoWPF::Core::SetInputRoot(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        std::uint32_t buttonState = static_cast<std::uint32_t>(wParam);
        if (uMsg == WM_RBUTTONDOWN)
            buttonState |= 0x02;
        else
            buttonState &= ~0x02;
        SubmitNativePointerInput(
            uMsg == WM_RBUTTONDOWN ? NeoWPF::Input::Core::PointerPhase::Down : NeoWPF::Input::Core::PointerPhase::Up,
            x, y, buttonState, 0x02);
        return 0;
    }

    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    {
        NeoWPF::Core::SetInputRoot(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        std::uint32_t buttonState = static_cast<std::uint32_t>(wParam);
        if (uMsg == WM_MBUTTONDOWN)
            buttonState |= 0x04;
        else
            buttonState &= ~0x04;
        SubmitNativePointerInput(
            uMsg == WM_MBUTTONDOWN ? NeoWPF::Input::Core::PointerPhase::Down : NeoWPF::Input::Core::PointerPhase::Up,
            x, y, buttonState, 0x04);
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        NeoWPF::Core::SetInputRoot(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        POINT screenPoint{ x, y };
        ScreenToClient(hwnd, &screenPoint);
        x = screenPoint.x;
        y = screenPoint.y;
        int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        std::uint32_t buttonState = GET_KEYSTATE_WPARAM(wParam);
        SubmitNativePointerInput(NeoWPF::Input::Core::PointerPhase::Wheel,
                     x, y, buttonState, 0, wheelDelta);
        return 0;
    }

    case WM_KEYDOWN:
    {
        NeoWPF::Core::SetInputRoot(hwnd);
        int keyCode = NeoWPF::Platform::Windows::TranslateVirtualKey(
            static_cast<std::uint32_t>(wParam));
        int repeatCount = static_cast<int>(lParam) & 0xFFFF;
        if (keyCode != static_cast<int>(NeoWPF::Input::Key::None)) {
            NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(keyCode, true, 0, repeatCount);
        }
        return uMsg == WM_SYSKEYDOWN ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : 0;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        NeoWPF::Core::SetInputRoot(hwnd);
        int keyCode = NeoWPF::Platform::Windows::TranslateVirtualKey(
            static_cast<std::uint32_t>(wParam));
        int repeatCount = static_cast<int>(lParam) & 0xFFFF;
        if (keyCode != static_cast<int>(NeoWPF::Input::Key::None)) {
            NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(keyCode, false, 0, repeatCount);
        }
        return uMsg == WM_SYSKEYUP ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : 0;
    }

    case WM_CHAR:
    case WM_SYSCHAR:
    {
        NeoWPF::Core::SetInputRoot(hwnd);
        wchar_t charCode = static_cast<wchar_t>(wParam);
        NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(0, true, charCode, 1);
        return uMsg == WM_SYSCHAR ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : 0;
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

namespace NeoWPF::Platform::Windows {

int TranslateVirtualKey(std::uint32_t virtualKey) noexcept
{
    return TranslateWindowsVirtualKey(static_cast<WPARAM>(virtualKey));
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

    auto& windows = NeoWPF::Core::GetAppState().windowSessions;
    for (auto& windowSession : windows) {
        if (windowSession.renderSurface.nativeWindowHandle == hwnd) {
            windowSession.contentRoot = content.get();
            windowSession.windowHost->SetContent(content);
            NeoWPF::Core::SetInputRoot(hwnd);
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

    const auto& windows = NeoWPF::Core::GetAppState().windowSessions;
    for (const auto& windowSession : windows) {
        if (windowSession.renderSurface.nativeWindowHandle == hwnd) {
            return windowSession.windowHost->GetContent();
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

    auto& windows = NeoWPF::Core::GetAppState().windowSessions;
    for (auto& windowSession : windows) {
        if (windowSession.renderSurface.nativeWindowHandle == hwnd) {
            windowSession.windowHost->SetTitleBar(titleBar);

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

    const auto& windows = NeoWPF::Core::GetAppState().windowSessions;
    for (const auto& windowSession : windows) {
        if (windowSession.renderSurface.nativeWindowHandle == hwnd) {
            return windowSession.windowHost->GetTitleBar();
        }
    }
    return nullptr;
}

void* Window::CreateWindow(void* hInst, const wchar_t* title, int w, int h, bool transparent)
{
    HINSTANCE hInstance = static_cast<HINSTANCE>(hInst);
    EnsureNeoWPFWindowClass(hInstance);
    const DWORD exStyle = transparent ? WS_EX_NOREDIRECTIONBITMAP : 0;
    const DWORD style = transparent ? WS_POPUP : WS_OVERLAPPEDWINDOW;
    HWND hwnd = CreateWindowExW(exStyle, NeoWPFWindowClassName, title, style, CW_USEDEFAULT, CW_USEDEFAULT, w, h, nullptr, nullptr, hInstance, nullptr);
    if (hwnd && transparent) {
        ::SetPropW(hwnd, L"NeoWPF.Transparent", reinterpret_cast<HANDLE>(1));
    }
    return hwnd;
}

}
