/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowChrome.h>

#include <functional>
#include <memory>

#undef CreateWindow

namespace NeoWPF::UI {
class Visual;
}

namespace NeoWPF::Platform {

class Window {
public:
    using ResizeCallback = std::function<void(int,int)>;

    Window() noexcept = default;
    explicit Window(void* nativeHandle) noexcept : m_nativeHandle(nativeHandle) {}
    void* Handle() const noexcept { return m_nativeHandle; }

    void Resize(int width, int height) const noexcept;
    void Show() const noexcept;
    void Hide() const noexcept;
    void Close() const noexcept;
    void Minimize() const noexcept;
    void ToggleMaximize() const noexcept;
    void SetChromeMode(NeoWPF::Contracts::ChromeMode mode) const noexcept;
    NeoWPF::Contracts::ChromeMode GetChromeMode() const noexcept;
    void SetResizeCallback(ResizeCallback cb) { m_onResize = std::move(cb); }

    void SetContent(std::shared_ptr<NeoWPF::UI::Visual> content) noexcept;
    std::shared_ptr<NeoWPF::UI::Visual> GetContent() const noexcept;
    void SetTitleBar(std::shared_ptr<NeoWPF::UI::Visual> titleBar) noexcept;
    std::shared_ptr<NeoWPF::UI::Visual> GetTitleBar() const noexcept;
    static void SetWindowContent(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> content);
    static std::shared_ptr<NeoWPF::UI::Visual> GetWindowContent(void* nativeHandle);
    static void SetWindowTitleBar(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> titleBar);
    static std::shared_ptr<NeoWPF::UI::Visual> GetWindowTitleBar(void* nativeHandle);

    /*
        @summary
        Creates a native window for the current platform.

        @param hInst
        Native platform instance handle.

        @param title
        Window title.

        @param w
        Initial window width.

        @param h
        Initial window height.

        @param transparent
        True to request a transparent native window.

        @returns
        The native window handle, or nullptr when creation fails.
    */
    static void* CreateWindow(void* hInst, const wchar_t* title, int w, int h, bool transparent = false);

private:
    void* m_nativeHandle = nullptr;
    ResizeCallback m_onResize;
    std::shared_ptr<NeoWPF::UI::Visual> m_content;
    std::shared_ptr<NeoWPF::UI::Visual> m_titleBar;
};

}
