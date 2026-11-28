#include <NeoWPF/Platform/Linux/Wayland/WlWindow.h>
#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <algorithm>
#include <cmath>

namespace NeoWPF::Platform::Linux {

namespace {

const xdg_surface_listener SurfaceListener = {
    &WlWindow::HandleSurfaceConfigure,
};

const xdg_toplevel_listener ToplevelListener = {
    &WlWindow::HandleToplevelConfigure,
    &WlWindow::HandleToplevelClose,
};

const wl_surface_listener WindowSurfaceListener = {
    &WlWindow::HandleSurfaceEnter,
    &WlWindow::HandleSurfaceLeave,
};

void HandlePreferredScale(void* data,
                          wp_fractional_scale_v1*,
                          std::uint32_t preferredScale)
{
    auto* window = static_cast<WlWindow*>(data);
    if (!window || preferredScale == 0) {
        return;
    }
    window->SetFractionalScaleFactor(static_cast<double>(preferredScale) / 120.0);
}

const wp_fractional_scale_v1_listener FractionalScaleListener = {
    &HandlePreferredScale,
};

}

WlWindow::WlWindow(WlWindowing& windowing, const wchar_t* title, int width, int height,
             bool transparent)
        : m_windowing(windowing),
            m_display(windowing.Display()),
      m_title(title ? title : L""),
      m_width((std::max)(width, 1)),
      m_height((std::max)(height, 1)),
    m_seat(windowing.Seat()),
    m_transparent(transparent)
{
    if (windowing.IsConnected() && windowing.Compositor()) {
        m_surface = wl_compositor_create_surface(windowing.Compositor());
        if (m_surface) {
            wl_surface_add_listener(m_surface, &WindowSurfaceListener, this);
            if (windowing.FractionalScaleManager()) {
                m_fractionalScale = wp_fractional_scale_manager_v1_get_fractional_scale(
                    windowing.FractionalScaleManager(), m_surface);
                if (m_fractionalScale) {
                    wp_fractional_scale_v1_add_listener(
                        m_fractionalScale, &FractionalScaleListener, this);
                }
            }
        }
        if (m_surface && windowing.XdgWmBase()) {
            m_xdgSurface = xdg_wm_base_get_xdg_surface(windowing.XdgWmBase(), m_surface);
            if (m_xdgSurface) {
                xdg_surface_add_listener(m_xdgSurface, &SurfaceListener, this);
                m_xdgToplevel = xdg_surface_get_toplevel(m_xdgSurface);
            }
            if (m_xdgToplevel) {
                xdg_toplevel_add_listener(m_xdgToplevel, &ToplevelListener, this);
                const auto titleUtf8 = NeoWPF::Platform::WideToUtf8(m_title);
                if (titleUtf8.Fail()) {
                    NeoWPF::Diagnostics::LogWarn(
                        "Wayland window title conversion failed: %s",
                        titleUtf8.Error().c_str());
                } else {
                    xdg_toplevel_set_title(m_xdgToplevel, titleUtf8.Value().c_str());
                }
                xdg_toplevel_set_app_id(m_xdgToplevel, "NeoWPF");
                wl_surface_commit(m_surface);
                wl_display_flush(m_display);
            }
        }
    }

    if (windowing.Seat()) {
        AttachInputDevices();
    }
}

WlWindow::~WlWindow()
{
    Close();
}

bool WlWindow::IsValid() const noexcept
{
    return m_surface != nullptr && m_xdgSurface != nullptr && m_xdgToplevel != nullptr;
}

void WlWindow::Close() noexcept
{
    if (m_fractionalScale) {
        wp_fractional_scale_v1_destroy(m_fractionalScale);
        m_fractionalScale = nullptr;
    }
    if (m_keyboard) {
        wl_keyboard_release(m_keyboard);
        m_keyboard = nullptr;
    }
    if (m_pointer) {
        wl_pointer_release(m_pointer);
        m_pointer = nullptr;
    }
    if (m_xkbState) {
        xkb_state_unref(m_xkbState);
        m_xkbState = nullptr;
    }
    if (m_xkbKeymap) {
        xkb_keymap_unref(m_xkbKeymap);
        m_xkbKeymap = nullptr;
    }
    if (m_xkbContext) {
        xkb_context_unref(m_xkbContext);
        m_xkbContext = nullptr;
    }
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

wl_surface* WlWindow::Surface() const noexcept
{
    return m_surface;
}

wl_display* WlWindow::Display() const noexcept
{
    return m_display;
}

const std::wstring& WlWindow::Title() const noexcept
{
    return m_title;
}

int WlWindow::Width() const noexcept
{
    return m_width;
}

int WlWindow::Height() const noexcept
{
    return m_height;
}

bool WlWindow::IsTransparent() const noexcept
{
    return m_transparent;
}

void WlWindow::SetSize(int width, int height) noexcept
{
    m_width = (std::max)(width, 1);
    m_height = (std::max)(height, 1);
}

bool WlWindow::IsConfigured() const noexcept
{
    return m_configured;
}

bool WlWindow::IsClosed() const noexcept
{
    return m_closed;
}

bool WlWindow::IsMaximized() const noexcept
{
    return m_maximized;
}

double WlWindow::ScaleFactor() const noexcept
{
    return m_scaleFactor;
}

const char* WlWindow::ScaleFactorUnavailableReason() const noexcept
{
    if (m_outputs.empty()) {
        return m_fractionalScale
            ? "waiting for the preferred fractional-scale event; the surface has not entered an output"
            : "the fractional-scale object is unavailable and the surface has not entered an output";
    }
    return "no usable integer scale is available for the entered outputs";
}

void WlWindow::SetFractionalScaleFactor(double scaleFactor) noexcept
{
    if (!std::isfinite(scaleFactor) || scaleFactor <= 0.0) {
        return;
    }
    if (m_scaleFactor != scaleFactor || !m_hasFractionalScale) {
        NeoWPF::Diagnostics::LogInfo(
            "[Wayland] Using fractional scale factor=%.2f", scaleFactor);
    }
    m_scaleFactor = scaleFactor;
    m_hasFractionalScale = true;
}

void WlWindow::UpdateIntegerScaleFactor() noexcept
{
    if (m_hasFractionalScale) {
        return;
    }

    int scaleFactor = 0;
    for (wl_output* output : m_outputs) {
        scaleFactor = (std::max)(scaleFactor, m_windowing.GetOutputScale(output));
    }
    if (scaleFactor <= 0) {
        return;
    }

    if (m_scaleFactor != static_cast<double>(scaleFactor) || m_hasFractionalScale) {
        NeoWPF::Diagnostics::LogInfo(
            "[Wayland] Using integer scale factor=%d", scaleFactor);
    }
    m_scaleFactor = static_cast<double>(scaleFactor);
    m_hasFractionalScale = false;
}

void WlWindow::RemoveOutput(wl_output* output) noexcept
{
    m_outputs.erase(std::remove(m_outputs.begin(), m_outputs.end(), output), m_outputs.end());
    UpdateIntegerScaleFactor();
}

void WlWindow::Minimize() noexcept
{
    if (m_xdgToplevel) {
        xdg_toplevel_set_minimized(m_xdgToplevel);
    }
}

void WlWindow::Maximize(bool maximize) noexcept
{
    if (!m_xdgToplevel) {
        return;
    }

    if (maximize) {
        xdg_toplevel_set_maximized(m_xdgToplevel);
    } else {
        xdg_toplevel_unset_maximized(m_xdgToplevel);
    }
}

void WlWindow::AcknowledgeConfigure() noexcept
{
    if (m_xdgSurface && m_configureSerial != 0) {
        xdg_surface_ack_configure(m_xdgSurface, m_configureSerial);
        m_configureSerial = 0;
    }
}

void WlWindow::HandleSurfaceConfigure(void* data, xdg_surface*, std::uint32_t serial)
{
    auto* window = static_cast<WlWindow*>(data);
    if (!window) {
        return;
    }
    window->m_configureSerial = serial;
    window->m_configured = true;
}

void WlWindow::HandleToplevelConfigure(void* data, xdg_toplevel*, int32_t width,
                                      int32_t height, wl_array*)
{
    auto* window = static_cast<WlWindow*>(data);
    if (!window) {
        return;
    }
    if (width > 0 && height > 0) {
        window->SetSize(width, height);
    }
}

void WlWindow::HandleToplevelClose(void* data, xdg_toplevel*)
{
    auto* window = static_cast<WlWindow*>(data);
    if (window) {
        window->m_closed = true;
    }
}

void WlWindow::HandleSurfaceEnter(void* data, wl_surface*, wl_output* output)
{
    auto* window = static_cast<WlWindow*>(data);
    if (!window || !output) {
        return;
    }
    if (std::find(window->m_outputs.begin(), window->m_outputs.end(), output) ==
        window->m_outputs.end()) {
        window->m_outputs.push_back(output);
    }
    window->UpdateIntegerScaleFactor();
}

void WlWindow::HandleSurfaceLeave(void* data, wl_surface*, wl_output* output)
{
    auto* window = static_cast<WlWindow*>(data);
    if (window && output) {
        window->RemoveOutput(output);
    }
}

}

namespace NeoWPF::Platform {

void Window::Resize(int width, int height) const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        platform->Windowing().ResizeWindow(m_nativeHandle, width, height);
    }
}

void Window::Show() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        platform->Windowing().ShowWindow(m_nativeHandle);
    }
}

void Window::Hide() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        platform->Windowing().HideWindow(m_nativeHandle);
    }
}

void Window::Close() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        platform->Windowing().CloseWindow(m_nativeHandle);
    }
}

void Window::Minimize() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        if (auto* chrome = platform->Chrome()) {
            chrome->SetMinimized(m_nativeHandle);
        }
    }
}

void Window::ToggleMaximize() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        if (auto* chrome = platform->Chrome()) {
            const bool maximized = chrome->IsMaximized(m_nativeHandle);
            chrome->SetMaximized(m_nativeHandle, !maximized);
        }
    }
}

void Window::SetChromeMode(NeoWPF::Contracts::ChromeMode mode) const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        if (auto* chrome = platform->Chrome()) {
            chrome->SetChromeMode(m_nativeHandle, mode);
        }
    }
}

NeoWPF::Contracts::ChromeMode Window::GetChromeMode() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        if (auto* chrome = platform->Chrome()) {
            return chrome->GetChromeMode(m_nativeHandle);
        }
    }
    return NeoWPF::Contracts::ChromeMode::Native;
}

void Window::SetContent(std::shared_ptr<NeoWPF::UI::Visual> content) noexcept
{
    SetWindowContent(m_nativeHandle, content);
    if (!m_nativeHandle) {
        m_content = std::move(content);
    }
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetContent() const noexcept
{
    if (m_nativeHandle) {
        return GetWindowContent(m_nativeHandle);
    }
    return m_content;
}

void Window::SetTitleBar(std::shared_ptr<NeoWPF::UI::Visual> titleBar) noexcept
{
    SetWindowTitleBar(m_nativeHandle, titleBar);
    if (!m_nativeHandle) {
        m_titleBar = std::move(titleBar);
    }
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetTitleBar() const noexcept
{
    if (m_nativeHandle) {
        return GetWindowTitleBar(m_nativeHandle);
    }
    return m_titleBar;
}

void Window::SetWindowContent(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> content)
{
    auto& app = NeoWPF::Core::GetAppState();
    std::shared_ptr<NeoWPF::UI::Window> host;
    {
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        for (auto& windowSession : app.windowSessions) {
            if (windowSession.renderSurface.nativeWindowHandle == nativeHandle && windowSession.windowHost) {
                windowSession.contentRoot = content.get();
                host = windowSession.windowHost;
                break;
            }
        }
    }
    if (host) {
        host->SetContent(std::move(content));
    }
    NeoWPF::Core::SetInputRoot(nativeHandle);
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetWindowContent(void* nativeHandle)
{
    auto& app = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
    for (const auto& windowSession : app.windowSessions) {
        if (windowSession.renderSurface.nativeWindowHandle == nativeHandle && windowSession.windowHost) {
            return windowSession.windowHost->GetContent();
        }
    }
    return nullptr;
}

void Window::SetWindowTitleBar(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> titleBar)
{
    auto& app = NeoWPF::Core::GetAppState();
    bool hasTitleBar = static_cast<bool>(titleBar);
    std::shared_ptr<NeoWPF::UI::Window> host;
    {
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        for (auto& windowSession : app.windowSessions) {
            if (windowSession.renderSurface.nativeWindowHandle == nativeHandle && windowSession.windowHost) {
                host = windowSession.windowHost;
                break;
            }
        }
    }
    if (host) {
        host->SetTitleBar(std::move(titleBar));
    }

    if (hasTitleBar) {
        if (auto* platform = CurrentPlatform()) {
            if (auto* chrome = platform->Chrome()) {
                chrome->SetChromeMode(nativeHandle, NeoWPF::Contracts::ChromeMode::Custom);
            }
        }
    }
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetWindowTitleBar(void* nativeHandle)
{
    auto& app = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
    for (const auto& windowSession : app.windowSessions) {
        if (windowSession.renderSurface.nativeWindowHandle == nativeHandle && windowSession.windowHost) {
            return windowSession.windowHost->GetTitleBar();
        }
    }
    return nullptr;
}

void* Window::CreateWindow(void*, const wchar_t* title, int width, int height, bool transparent)
{
    auto* platform = CurrentPlatform();
    return platform ? platform->Windowing().CreateWindow(title, width, height) : nullptr;
}

}
