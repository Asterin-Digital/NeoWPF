#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Input/InputManager.h>
#include <windows.h>
#include <windowsx.h>

namespace {

const wchar_t* NeoWPFWindowClassName = L"NeoWPFWindowClass";

LRESULT CALLBACK NeoWPFWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        NeoWPF::Core::SetInputRootForWindow(hwnd);
        return 0;

    case WM_DESTROY:
        return 0;

    case WM_CLOSE:
        NeoWPF::Core::CloseApplicationWindow(hwnd);
        return 0;

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

void* Window::CreateWindow(void* hInst, const wchar_t* title, int w, int h)
{
    HINSTANCE hInstance = static_cast<HINSTANCE>(hInst);
    EnsureNeoWPFWindowClass(hInstance);
    HWND hwnd = CreateWindowExW(0, NeoWPFWindowClassName, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, nullptr, nullptr, hInstance, nullptr);
    return hwnd;
}

}
