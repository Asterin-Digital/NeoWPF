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

/*
    @summary
    Controls a native window and its hosted visuals.

    @internal
*/
class Window {
public:
    /*
        @summary
        Callback invoked with the new size after a window resize.

        @internal
    */
    using ResizeCallback = std::function<void(int,int)>;

    Window() noexcept = default;
    explicit Window(void* nativeHandle) noexcept : m_nativeHandle(nativeHandle) {}
    /*
        @summary
        Returns the native window handle.

        @internal
    */
    void* Handle() const noexcept { return m_nativeHandle; }

    /*
        @summary
        Resizes the native window.

        @param width
        New window width.

        @param height
        New window height.

        @internal
    */
    void Resize(int width, int height) const noexcept;
    /*
        @summary
        Shows the native window.

        @internal
    */
    void Show() const noexcept;
    /*
        @summary
        Hides the native window.

        @internal
    */
    void Hide() const noexcept;
    /*
        @summary
        Closes the native window.

        @internal
    */
    void Close() const noexcept;
    /*
        @summary
        Minimizes the native window.

        @internal
    */
    void Minimize() const noexcept;
    /*
        @summary
        Toggles the native window between maximized and restored states.

        @internal
    */
    void ToggleMaximize() const noexcept;
    /*
        @summary
        Sets the native window chrome mode.

        @param mode
        Chrome mode to apply.

        @internal
    */
    void SetChromeMode(NeoWPF::Contracts::ChromeMode mode) const noexcept;
    /*
        @summary
        Returns the current native window chrome mode.

        @internal
    */
    NeoWPF::Contracts::ChromeMode GetChromeMode() const noexcept;
    /*
        @summary
        Sets the callback invoked when the window is resized.

        @param cb
        Resize callback.

        @internal
    */
    void SetResizeCallback(ResizeCallback cb) { m_onResize = std::move(cb); }

    /*
        @summary
        Sets the visual content hosted by the window.

        @param content
        Window content visual.

        @internal
    */
    void SetContent(std::shared_ptr<NeoWPF::UI::Visual> content) noexcept;
    /*
        @summary
        Returns the visual content hosted by the window.

        @internal
    */
    std::shared_ptr<NeoWPF::UI::Visual> GetContent() const noexcept;
    /*
        @summary
        Sets the visual hosted in the window title bar.

        @param titleBar
        Title bar visual.

        @internal
    */
    void SetTitleBar(std::shared_ptr<NeoWPF::UI::Visual> titleBar) noexcept;
    /*
        @summary
        Returns the visual hosted in the window title bar.

        @internal
    */
    std::shared_ptr<NeoWPF::UI::Visual> GetTitleBar() const noexcept;
    /*
        @summary
        Sets the content visual associated with a native handle.

        @param nativeHandle
        Native window handle.

        @param content
        Content visual to associate.

        @internal
    */
    static void SetWindowContent(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> content);
    /*
        @summary
        Returns the content visual associated with a native handle.

        @param nativeHandle
        Native window handle.

        @internal
    */
    static std::shared_ptr<NeoWPF::UI::Visual> GetWindowContent(void* nativeHandle);
    /*
        @summary
        Sets the title bar visual associated with a native handle.

        @param nativeHandle
        Native window handle.

        @param titleBar
        Title bar visual to associate.

        @internal
    */
    static void SetWindowTitleBar(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> titleBar);
    /*
        @summary
        Returns the title bar visual associated with a native handle.

        @param nativeHandle
        Native window handle.

        @internal
    */
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

        @internal
    */
    static void* CreateWindow(void* hInst, const wchar_t* title, int w, int h, bool transparent = false);

private:
    void* m_nativeHandle = nullptr;
    ResizeCallback m_onResize;
    std::shared_ptr<NeoWPF::UI::Visual> m_content;
    std::shared_ptr<NeoWPF::UI::Visual> m_titleBar;
};

}
