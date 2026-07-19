#pragma once

#include <Windows.h>
#undef CreateWindow
#include <functional>
#include <memory>

namespace NeoWPF::UI {
class Visual;
}

namespace NeoWPF::Platform {

class Window {
public:
    using ResizeCallback = std::function<void(int,int)>;

    Window() noexcept = default;
    explicit Window(HWND hwnd) noexcept : m_hwnd(hwnd) {}
    HWND Handle() const noexcept { return m_hwnd; }
    void Resize(int width, int height) const noexcept {
        if (m_hwnd) {
            SetWindowPos(m_hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
        }
    }
    void Close() const noexcept {
        if (m_hwnd) {
            PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
        }
    }
    void SetResizeCallback(ResizeCallback cb) { m_onResize = std::move(cb); }

    void SetContent(std::shared_ptr<NeoWPF::UI::Visual> content) noexcept;
    std::shared_ptr<NeoWPF::UI::Visual> GetContent() const noexcept;
    static void SetWindowContent(HWND hwnd, std::shared_ptr<NeoWPF::UI::Visual> content);
    static std::shared_ptr<NeoWPF::UI::Visual> GetWindowContent(HWND hwnd);

    static HWND CreateWindow(HINSTANCE hInst, const wchar_t* title, int w, int h);

private:
    HWND m_hwnd = nullptr;
    ResizeCallback m_onResize;
    std::shared_ptr<NeoWPF::UI::Visual> m_content;
};

}
