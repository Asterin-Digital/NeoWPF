#pragma once

#include <NeoWPF/Platform/Linux/LWindowing.h>

#include <string>

namespace NeoWPF::Platform::Linux {

class LWindow final {
public:
    LWindow(LWindowing& windowing, const wchar_t* title, int width, int height);
    ~LWindow();

    LWindow(const LWindow&) = delete;
    LWindow& operator=(const LWindow&) = delete;

    bool IsValid() const noexcept;
    void Close() noexcept;

    wl_surface* Surface() const noexcept;
    wl_display* Display() const noexcept;
    const std::wstring& Title() const noexcept;
    int Width() const noexcept;
    int Height() const noexcept;
    void SetSize(int width, int height) noexcept;
    bool IsConfigured() const noexcept;
    bool IsClosed() const noexcept;
    void AcknowledgeConfigure() noexcept;

    static void HandleSurfaceConfigure(void* data, xdg_surface* surface, std::uint32_t serial);
    static void HandleToplevelConfigure(void* data, xdg_toplevel* toplevel, int32_t width,
                                        int32_t height, wl_array* states);
    static void HandleToplevelClose(void* data, xdg_toplevel* toplevel);

private:
    wl_display* m_display = nullptr;
    wl_surface* m_surface = nullptr;
    xdg_surface* m_xdgSurface = nullptr;
    xdg_toplevel* m_xdgToplevel = nullptr;
    zxdg_toplevel_decoration_v1* m_xdgDecoration = nullptr;
    std::wstring m_title;
    int m_width = 0;
    int m_height = 0;
    std::uint32_t m_configureSerial = 0;
    bool m_configured = false;
    bool m_closed = false;
};

}
