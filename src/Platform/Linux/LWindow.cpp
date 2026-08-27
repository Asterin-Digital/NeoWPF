#include <NeoWPF/Platform/Linux/LWindow.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>
#include <string>

namespace NeoWPF::Platform::Linux {

namespace {

const xdg_surface_listener SurfaceListener = {
    &LWindow::HandleSurfaceConfigure,
};

const xdg_toplevel_listener ToplevelListener = {
    &LWindow::HandleToplevelConfigure,
    &LWindow::HandleToplevelClose,
};

}

LWindow::LWindow(LWindowing& windowing, const wchar_t* title, int width, int height)
    : m_display(windowing.Display()),
      m_title(title ? title : L""),
      m_width((std::max)(width, 1)),
      m_height((std::max)(height, 1))
{
    if (windowing.IsConnected() && windowing.Compositor()) {
        m_surface = wl_compositor_create_surface(windowing.Compositor());
        if (m_surface && windowing.XdgWmBase()) {
            m_xdgSurface = xdg_wm_base_get_xdg_surface(windowing.XdgWmBase(), m_surface);
            if (m_xdgSurface) {
                xdg_surface_add_listener(m_xdgSurface, &SurfaceListener, this);
                m_xdgToplevel = xdg_surface_get_toplevel(m_xdgSurface);
            }
            if (m_xdgToplevel) {
                xdg_toplevel_add_listener(m_xdgToplevel, &ToplevelListener, this);
                const auto titleUtf8 = NeoWPF::Platform::WideToUtf8(m_title);
                xdg_toplevel_set_title(m_xdgToplevel, titleUtf8.c_str());
                xdg_toplevel_set_app_id(m_xdgToplevel, "NeoWPF");
                wl_surface_commit(m_surface);
                wl_display_flush(m_display);
            }
        }
    }
}

LWindow::~LWindow()
{
    Close();
}

bool LWindow::IsValid() const noexcept
{
    return m_surface != nullptr && m_xdgSurface != nullptr && m_xdgToplevel != nullptr;
}

void LWindow::Close() noexcept
{
    if (m_xdgToplevel) {
        xdg_toplevel_destroy(m_xdgToplevel);
        m_xdgToplevel = nullptr;
    }
    if (m_xdgSurface) {
        xdg_surface_destroy(m_xdgSurface);
        m_xdgSurface = nullptr;
    }
    if (m_surface) {
        wl_surface_destroy(m_surface);
        m_surface = nullptr;
    }
}

wl_surface* LWindow::Surface() const noexcept
{
    return m_surface;
}

wl_display* LWindow::Display() const noexcept
{
    return m_display;
}

const std::wstring& LWindow::Title() const noexcept
{
    return m_title;
}

int LWindow::Width() const noexcept
{
    return m_width;
}

int LWindow::Height() const noexcept
{
    return m_height;
}

void LWindow::SetSize(int width, int height) noexcept
{
    m_width = (std::max)(width, 1);
    m_height = (std::max)(height, 1);
}

bool LWindow::IsConfigured() const noexcept
{
    return m_configured;
}

bool LWindow::IsClosed() const noexcept
{
    return m_closed;
}

void LWindow::AcknowledgeConfigure() noexcept
{
    if (m_xdgSurface && m_configureSerial != 0) {
        xdg_surface_ack_configure(m_xdgSurface, m_configureSerial);
        m_configureSerial = 0;
    }
}

void LWindow::HandleSurfaceConfigure(void* data, xdg_surface*, std::uint32_t serial)
{
    auto* window = static_cast<LWindow*>(data);
    window->m_configureSerial = serial;
    window->m_configured = true;
}

void LWindow::HandleToplevelConfigure(void* data, xdg_toplevel*, int32_t width,
                                      int32_t height, wl_array*)
{
    auto* window = static_cast<LWindow*>(data);
    if (width > 0 && height > 0) {
        window->SetSize(width, height);
    }
}

void LWindow::HandleToplevelClose(void* data, xdg_toplevel*)
{
    static_cast<LWindow*>(data)->m_closed = true;
}

}
