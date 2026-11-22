/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Platform/Linux/X11/XWindowing.h>

#include <X11/Xlib.h>

#undef Success
#undef None

#include <chrono>
#include <cstdint>
#include <string>

namespace NeoWPF::Platform::Linux {

class XWindow final {
public:
    XWindow(XWindowing& windowing, const wchar_t* title, int width, int height, bool transparent);
    ~XWindow();

    XWindow(const XWindow&) = delete;
    XWindow& operator=(const XWindow&) = delete;

    bool IsValid() const noexcept;
    void Close() noexcept;
    void Resize(int width, int height) noexcept;
    void Show() noexcept;
    void Hide() noexcept;
    void Minimize() noexcept;
    void Maximize(bool maximize) noexcept;
    bool IsClosed() const noexcept;
    bool IsMaximized() const noexcept;
    int Width() const noexcept;
    int Height() const noexcept;
    double ScaleFactor() const noexcept;
    void* NativeHandle() const noexcept;
    ::Window XWindowHandle() const noexcept;
    Display* DisplayHandle() const noexcept;
    void HandleEvent(XEvent& event) noexcept;
    void ProcessKeyRepeat() noexcept;

private:
    XWindowing& m_windowing;
    Display* m_display = nullptr;
    ::Window m_window = 0;
    std::wstring m_title;
    int m_width = 1;
    int m_height = 1;
    double m_scaleFactor = 1.0;
    bool m_closed = false;
    bool m_maximized = false;
    bool m_transparent = false;
    std::uint32_t m_repeatKey = 0;
    wchar_t m_repeatCharacter = L'\0';
    bool m_repeatActive = false;
    std::chrono::steady_clock::time_point m_repeatNext{};
};

}
