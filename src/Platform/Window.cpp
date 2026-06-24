#include <NeoWPF/Platform/Window.h>
#include <windowsx.h>

namespace {

const wchar_t* NeoWPFWindowClassName = L"NeoWPFWindowClass";

LRESULT CALLBACK NeoWPFWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
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
