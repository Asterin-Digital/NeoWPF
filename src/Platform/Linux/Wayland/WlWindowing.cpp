#include <NeoWPF/Platform/Linux/Wayland/WlWindowing.h>
#include <NeoWPF/Platform/Linux/LClientSideTitleBar.h>
#include <NeoWPF/Platform/Linux/Wayland/WlWindow.h>
#include <NeoWPF/Platform/Linux/Wayland/WlClipboard.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/UI/Controls/Window.h>

#ifndef VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#include <vulkan/vulkan.h>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#include <xdg-shell-protocol.cpp>
#include <fractional-scale-v1-protocol.cpp>
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <poll.h>

namespace NeoWPF::Platform::Linux {
namespace {

const char* VulkanInstanceExtensions[] = {
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
};

bool CreateVulkanSurface(void*, void* instance, void* window, void* display,
                         void* outSurface) noexcept
{
    const VkWaylandSurfaceCreateInfoKHR createInfo{
        VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        nullptr,
        0,
        static_cast<wl_display*>(display),
        static_cast<wl_surface*>(window),
    };
    const VkResult result = vkCreateWaylandSurfaceKHR(
        static_cast<VkInstance>(instance), &createInfo, nullptr,
        static_cast<VkSurfaceKHR*>(outSurface));
    NeoWPF::Diagnostics::LogInfo(
        "[Vulkan/Wayland] vkCreateWaylandSurfaceKHR: instance=%p display=%p surface=%p result=%d",
        instance, display, window, static_cast<int>(result));
    return result == VK_SUCCESS;
}

const wl_registry_listener RegistryListener = {
    &WlWindowing::HandleGlobal,
    &WlWindowing::HandleGlobalRemove,
};

void HandleOutputGeometry(void*, wl_output*, int32_t, int32_t, int32_t, int32_t,
                          int32_t, const char*, const char*, int32_t)
{
}

void HandleOutputMode(void*, wl_output*, std::uint32_t, int32_t, int32_t, int32_t)
{
}

void HandleOutputDone(void*, wl_output*)
{
}

const wl_output_listener OutputListener = {
    &HandleOutputGeometry,
    &HandleOutputMode,
    &HandleOutputDone,
    &WlWindowing::HandleOutputScale,
};

void HandleXdgWmBasePing(void* data, xdg_wm_base* shell, std::uint32_t serial)
{
    auto* windowing = static_cast<WlWindowing*>(data);
    xdg_wm_base_pong(shell, serial);
    if (windowing->Display()) {
        wl_display_flush(windowing->Display());
    }
}

const xdg_wm_base_listener XdgWmBaseListener = {
    &HandleXdgWmBasePing,
};

}

WlWindowing::~WlWindowing()
{
    Disconnect();
}

bool WlWindowing::Connect()
{
    if (IsConnected()) {
        return true;
    }

    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        return false;
    }

    m_registry = wl_display_get_registry(m_display);
    if (!m_registry) {
        Disconnect();
        return false;
    }

    if (wl_registry_add_listener(m_registry, &RegistryListener, this) != 0 ||
        wl_display_roundtrip(m_display) < 0) {
        Disconnect();
        return false;
    }

    return m_compositor != nullptr && m_hasXdgShell;
}

void* WlWindowing::CreateWindow(
    const wchar_t* title,
    int width,
    int height,
    std::shared_ptr<NeoWPF::UI::Window> windowHost)
{
    if (!Connect()) {
        NeoWPF::Diagnostics::Catastrophic("Wayland connection or xdg-shell setup failed");
        return nullptr;
    }

    const bool transparent = windowHost && windowHost->IsTransparent();
    auto window = std::make_unique<WlWindow>(*this, title, width, height, transparent);
    if (!window->IsValid()) {
        return nullptr;
    }
    if (wl_display_roundtrip(m_display) < 0 || !window->IsConfigured()) {
        return nullptr;
    }
    window->AcknowledgeConfigure();

    void* nativeHandle = window.get();
    auto& app = NeoWPF::Core::GetAppState();
    NeoWPF::Core::WindowSession windowSession;
    windowSession.renderSurface.nativeWindowHandle = nativeHandle;
    windowSession.width = static_cast<unsigned int>(window->Width());
    windowSession.height = static_cast<unsigned int>(window->Height());
    windowSession.needsLayout = true;
    windowSession.windowHost = windowHost ? std::move(windowHost) : std::make_shared<NeoWPF::UI::Window>();

    {
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        windowSession.lifecycle.Activate();
        app.windowSessions.push_back(windowSession);
        app.hasCreatedWindow = true;
    }
    m_windows.push_back(std::move(window));

    if (!transparent) {
        EnsureClientTitleBar(
            nativeHandle,
            windowSession.windowHost->GetTitleBar()
                ? NeoWPF::Contracts::ChromeMode::Custom
                : NeoWPF::Contracts::ChromeMode::Native);
    }

    return nativeHandle;
}

bool WlWindowing::GetNativeWindowInfo(void* nativeWindowHandle,
                                     NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept
{
    outInfo = {};
    const auto* window = FindWindow(nativeWindowHandle);
    if (!window || !window->IsValid()) {
        return false;
    }
    outInfo.system = NeoWPF::Contracts::NativeWindowSystem::Wayland;
    outInfo.surfaceAlpha = window->IsTransparent()
        ? NeoWPF::Contracts::SurfaceAlpha::Premultiplied
        : NeoWPF::Contracts::SurfaceAlpha::Opaque;
    outInfo.window = window->Surface();
    outInfo.display = window->Display();
    outInfo.vulkanInstanceExtensions = VulkanInstanceExtensions;
    outInfo.vulkanInstanceExtensionCount = std::size(VulkanInstanceExtensions);
    outInfo.createVulkanSurface = &CreateVulkanSurface;
    return true;
}

bool WlWindowing::IsWindowAlive(void* nativeWindowHandle) const noexcept
{
    const auto* window = FindWindow(nativeWindowHandle);
    return window && window->IsValid() && !window->IsClosed();
}

void WlWindowing::GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth,
                               unsigned int& outHeight) const noexcept
{
    outWidth = 0;
    outHeight = 0;
    const auto* window = FindWindow(nativeWindowHandle);
    if (window && window->IsConfigured() && !window->IsClosed()) {
        outWidth = static_cast<unsigned int>(window->Width());
        outHeight = static_cast<unsigned int>(window->Height());
    }
}

bool WlWindowing::GetWindowScaleFactor(void* nativeWindowHandle,
                                      double& outScaleFactor) const noexcept
{
    outScaleFactor = 0.0;
    const auto* window = FindWindow(nativeWindowHandle);
    if (!window) {
        NeoWPF::Diagnostics::LogWarn(
            "[Wayland] Window scale unavailable: native window handle is not registered");
        return false;
    }
    if (window->IsClosed()) {
        NeoWPF::Diagnostics::LogWarn(
            "[Wayland] Window scale unavailable: window is closed");
        return false;
    }
    if (!window->IsConfigured()) {
        NeoWPF::Diagnostics::LogWarn(
            "[Wayland] Window scale unavailable: waiting for the initial xdg-surface configure");
        return false;
    }
    if (!std::isfinite(window->ScaleFactor()) || window->ScaleFactor() <= 0.0) {
        NeoWPF::Diagnostics::LogWarn(
            "[Wayland] Window scale unavailable: %s",
            window->ScaleFactorUnavailableReason());
        return false;
    }
    outScaleFactor = window->ScaleFactor();
    return true;
}

bool WlWindowing::GetWindowInsets(void* nativeWindowHandle,
                                  NeoWPF::Contracts::WindowInsets& outSafeArea,
                                  NeoWPF::Contracts::WindowInsets& outKeyboard) const noexcept
{
    if (!FindWindow(nativeWindowHandle)) {
        return false;
    }
    outSafeArea = {};
    outKeyboard = {};
    return true;
}

bool WlWindowing::GetInputProfile(void* nativeWindowHandle,
                                  NeoWPF::Contracts::InputProfile& outProfile) const noexcept
{
    if (!FindWindow(nativeWindowHandle)) {
        return false;
    }
    outProfile.primaryPointer = NeoWPF::Contracts::PointerPrecision::Fine;
    outProfile.canHover = true;
    outProfile.hasTouch = false;
    return true;
}

void WlWindowing::ResizeWindow(void* nativeWindowHandle, int width, int height) noexcept
{
    if (auto* window = FindWindow(nativeWindowHandle)) {
        window->SetSize(width, height);
        wl_surface_commit(window->Surface());
        wl_display_flush(window->Display());
    }
}

void WlWindowing::ShowWindow(void* nativeWindowHandle) noexcept
{
    if (auto* window = FindWindow(nativeWindowHandle)) {
        wl_surface_commit(window->Surface());
        wl_display_flush(window->Display());
    }
}

void WlWindowing::HideWindow(void*) noexcept
{
}

void WlWindowing::CloseWindow(void* nativeWindowHandle) noexcept
{
    auto it = std::find_if(m_windows.begin(), m_windows.end(), [nativeWindowHandle](const auto& window) {
        return window.get() == nativeWindowHandle;
    });
    if (it == m_windows.end()) {
        return;
    }
    NeoWPF::Core::CloseWindow(nativeWindowHandle);
    (*it)->Close();
    m_windows.erase(it);
}

void WlWindowing::InvalidateWindow(void* nativeWindowHandle) noexcept
{
    ShowWindow(nativeWindowHandle);
}

bool WlWindowing::PumpMessages(std::chrono::milliseconds timeout)
{
    if (!m_display || wl_display_dispatch_pending(m_display) < 0) {
        return false;
    }
    if (m_clipboard) {
        m_clipboard->ProcessPendingReads();
    }
    wl_display_flush(m_display);
    pollfd descriptor{wl_display_get_fd(m_display), POLLIN, 0};
    const int result = poll(&descriptor, 1, static_cast<int>(timeout.count()));
    if (result < 0) {
        return false;
    }
    if (result > 0 && wl_display_dispatch(m_display) < 0) {
        return false;
    }
    for (auto& window : m_windows) {
        window->ProcessKeyRepeat();
        if (window->IsClosed()) {
            void* closedWindow = window.get();
            CloseWindow(closedWindow);
            break;
        }
    }
    return !m_windows.empty() && m_exitCode == 0;
}

int WlWindowing::ExitCode() const noexcept
{
    return m_exitCode;
}

void WlWindowing::RequestQuit(int exitCode) noexcept
{
    m_exitCode = exitCode;
    while (!m_windows.empty()) {
        CloseWindow(m_windows.back().get());
    }
}

void WlWindowing::ShowFatalError(const std::wstring&, const std::wstring& message) noexcept
{
    NeoWPF::Diagnostics::Catastrophic("%s", NeoWPF::Platform::WideToUtf8(message).c_str());
}

void WlWindowing::SetConsoleVisible(bool visible) noexcept
{
    m_consoleVisible = visible;
}

bool WlWindowing::IsConsoleVisible() const noexcept
{
    return m_consoleVisible;
}

WlWindow* WlWindowing::FindWindow(void* nativeWindowHandle) noexcept
{
    const auto it = std::find_if(m_windows.begin(), m_windows.end(), [nativeWindowHandle](const auto& window) {
        return window.get() == nativeWindowHandle;
    });
    return it == m_windows.end() ? nullptr : it->get();
}

const WlWindow* WlWindowing::FindWindow(void* nativeWindowHandle) const noexcept
{
    const auto it = std::find_if(m_windows.begin(), m_windows.end(), [nativeWindowHandle](const auto& window) {
        return window.get() == nativeWindowHandle;
    });
    return it == m_windows.end() ? nullptr : it->get();
}

void WlWindowing::Disconnect() noexcept
{
    for (auto& window : m_windows) {
        window->Close();
    }
    m_windows.clear();
    if (m_compositor) {
        wl_compositor_destroy(m_compositor);
        m_compositor = nullptr;
    }
    if (m_shm) {
        wl_shm_destroy(m_shm);
        m_shm = nullptr;
    }
    if (m_dataDeviceManager) {
        wl_data_device_manager_destroy(m_dataDeviceManager);
        m_dataDeviceManager = nullptr;
    }
    if (m_xdgWmBase) {
        xdg_wm_base_destroy(m_xdgWmBase);
        m_xdgWmBase = nullptr;
    }
    if (m_fractionalScaleManager) {
        wp_fractional_scale_manager_v1_destroy(m_fractionalScaleManager);
        m_fractionalScaleManager = nullptr;
    }
    for (auto& output : m_outputs) {
        if (output.output) {
            wl_output_destroy(output.output);
        }
    }
    m_outputs.clear();
    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }
    if (m_display) {
        wl_display_disconnect(m_display);
        m_display = nullptr;
    }
    m_hasXdgShell = false;
}

bool WlWindowing::IsConnected() const noexcept
{
    return m_display != nullptr && m_registry != nullptr;
}

wl_display* WlWindowing::Display() const noexcept
{
    return m_display;
}

wl_compositor* WlWindowing::Compositor() const noexcept
{
    return m_compositor;
}

wl_shm* WlWindowing::Shm() const noexcept
{
    return m_shm;
}

wl_seat* WlWindowing::Seat() const noexcept
{
    return m_seat;
}

wl_data_device_manager* WlWindowing::DataDeviceManager() const noexcept
{
    return m_dataDeviceManager;
}

int WlWindowing::GetOutputScale(wl_output* output) const noexcept
{
    const auto it = std::find_if(
        m_outputs.begin(), m_outputs.end(),
        [output](const OutputState& state) { return state.output == output; });
    return it == m_outputs.end() ? 0 : it->scale;
}

void WlWindowing::AttachClipboard(WlClipboard* clipboard) noexcept
{
    m_clipboard = clipboard;
}

void WlWindowing::DetachClipboard(WlClipboard* clipboard) noexcept
{
    if (m_clipboard == clipboard) {
        m_clipboard = nullptr;
    }
}

void WlWindowing::RegisterInputSerial(std::uint32_t serial) noexcept
{
    if (serial != 0) {
        m_lastInputSerial = serial;
    }
}

std::uint32_t WlWindowing::LastInputSerial() const noexcept
{
    return m_lastInputSerial;
}

xdg_wm_base* WlWindowing::XdgWmBase() const noexcept
{
    return m_xdgWmBase;
}

wp_fractional_scale_manager_v1*
WlWindowing::FractionalScaleManager() const noexcept
{
    return m_fractionalScaleManager;
}

bool WlWindowing::HasXdgShell() const noexcept
{
    return m_hasXdgShell;
}

void WlWindowing::HandleGlobal(void* data, wl_registry* registry, std::uint32_t name,
                              const char* interface, std::uint32_t version)
{
    auto* windowing = static_cast<WlWindowing*>(data);
    if (std::strcmp(interface, wl_output_interface.name) == 0) {
        auto* output = static_cast<wl_output*>(wl_registry_bind(
            registry, name, &wl_output_interface, std::min(version, 2u)));
        if (output) {
            wl_output_add_listener(output, &OutputListener, windowing);
            windowing->m_outputs.push_back({name, output, 1});
        }
    } else if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        const auto bindVersion = std::min(version, 4u);
        windowing->m_compositor = static_cast<wl_compositor*>(wl_registry_bind(
            registry, name, &wl_compositor_interface, bindVersion));
    } else if (std::strcmp(interface, wl_shm_interface.name) == 0) {
        windowing->m_shm = static_cast<wl_shm*>(wl_registry_bind(
            registry, name, &wl_shm_interface, 1));
    } else if (std::strcmp(interface, wl_seat_interface.name) == 0) {
        windowing->m_seat = static_cast<wl_seat*>(wl_registry_bind(
            registry, name, &wl_seat_interface, 1));
        if (windowing->m_seat) {
            NeoWPF::Diagnostics::LogInfo("[Wayland] Connected to seat");
        }
    } else if (std::strcmp(interface, wl_data_device_manager_interface.name) == 0) {
        windowing->m_dataDeviceManager = static_cast<wl_data_device_manager*>(wl_registry_bind(
            registry, name, &wl_data_device_manager_interface, std::min(version, 3u)));
    } else if (std::strcmp(interface, "xdg_wm_base") == 0) {
        windowing->m_xdgWmBase = static_cast<xdg_wm_base*>(wl_registry_bind(
            registry, name, &xdg_wm_base_interface, 1));
        if (windowing->m_xdgWmBase) {
            xdg_wm_base_add_listener(windowing->m_xdgWmBase, &XdgWmBaseListener, windowing);
            windowing->m_hasXdgShell = true;
        }
    } else if (std::strcmp(interface,
                          wp_fractional_scale_manager_v1_interface.name) == 0) {
        windowing->m_fractionalScaleManager =
            static_cast<wp_fractional_scale_manager_v1*>(wl_registry_bind(
                registry, name, &wp_fractional_scale_manager_v1_interface,
                std::min(version, 1u)));
    }
}

void WlWindowing::HandleGlobalRemove(void* data, wl_registry*, std::uint32_t name)
{
    auto* windowing = static_cast<WlWindowing*>(data);
    const auto outputIt = std::find_if(
        windowing->m_outputs.begin(), windowing->m_outputs.end(),
        [name](const OutputState& output) { return output.name == name; });
    if (outputIt == windowing->m_outputs.end()) {
        return;
    }

    wl_output* output = outputIt->output;
    windowing->m_outputs.erase(outputIt);
    for (auto& window : windowing->m_windows) {
        window->RemoveOutput(output);
    }
    wl_output_destroy(output);
}

void WlWindowing::HandleOutputScale(void* data, wl_output* output, int32_t scale)
{
    auto* windowing = static_cast<WlWindowing*>(data);
    if (!windowing || scale <= 0) {
        return;
    }
    const auto outputIt = std::find_if(
        windowing->m_outputs.begin(), windowing->m_outputs.end(),
        [output](const OutputState& state) { return state.output == output; });
    if (outputIt == windowing->m_outputs.end()) {
        return;
    }

    outputIt->scale = scale;
    for (auto& window : windowing->m_windows) {
        window->UpdateIntegerScaleFactor();
    }
}

}