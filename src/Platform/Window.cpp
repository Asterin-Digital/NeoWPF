#include <NeoWPF/Platform/Window.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Input/InputManager.h>
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

HWND Window::CreateWindow(HINSTANCE hInst, const wchar_t* title, int w, int h)
{
    EnsureNeoWPFWindowClass(hInst);
    HWND hwnd = CreateWindowExW(0, NeoWPFWindowClassName, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, nullptr, nullptr, hInst, nullptr);
    return hwnd;
}

}
