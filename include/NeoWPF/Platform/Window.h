#pragma once

#include <Windows.h>
#undef CreateWindow
#include <functional>

namespace NeoWPF::Platform {

class Window {
public:
    using ResizeCallback = std::function<void(int,int)>;

    explicit Window(HWND hwnd) noexcept : m_hwnd(hwnd) {}
    HWND Handle() const noexcept { return m_hwnd; }
    void SetResizeCallback(ResizeCallback cb) { m_onResize = std::move(cb); }

    static HWND CreateWindow(HINSTANCE hInst, const wchar_t* title, int w, int h);

private:
    HWND m_hwnd = nullptr;
    ResizeCallback m_onResize;
};

}
