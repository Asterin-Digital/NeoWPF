#include <NeoWPF/Platform/Linux/LWindowing.h>
#include <NeoWPF/Platform/Linux/LClientSideTitleBar.h>
#include <NeoWPF/Platform/Linux/LWindow.h>
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
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <cstdint>
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
    &LWindowing::HandleGlobal,
    &LWindowing::HandleGlobalRemove,
};

void HandleXdgWmBasePing(void* data, xdg_wm_base* shell, std::uint32_t serial)
{
    auto* windowing = static_cast<LWindowing*>(data);
    xdg_wm_base_pong(shell, serial);
    if (windowing->Display()) {
        wl_display_flush(windowing->Display());
    }
}

const xdg_wm_base_listener XdgWmBaseListener = {
    &HandleXdgWmBasePing,
};

}

LWindowing::~LWindowing()
{
    Disconnect();
}

bool LWindowing::Connect()
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

void* LWindowing::CreateWindow(const wchar_t* title, int width, int height)
{
    if (!Connect()) {
        NeoWPF::Diagnostics::LogError("Wayland connection or xdg-shell setup failed");
        return nullptr;
    }

    auto window = std::make_unique<LWindow>(*this, title, width, height);
    if (!window->IsValid()) {
        return nullptr;
    }
    if (wl_display_roundtrip(m_display) < 0 || !window->IsConfigured()) {
        return nullptr;
    }
    window->AcknowledgeConfigure();

    void* nativeHandle = window.get();
    auto& app = NeoWPF::Core::GetAppState();
    NeoWPF::Core::WindowState windowState;
    windowState.nativeWindowHandle = nativeHandle;
    windowState.width = static_cast<unsigned int>(window->Width());
    windowState.height = static_cast<unsigned int>(window->Height());
    windowState.needsLayout = true;
    windowState.isPrimaryWindow = app.windows.empty();
    windowState.windowHost = std::make_shared<NeoWPF::UI::Window>();

    {
        std::lock_guard<std::mutex> lock(app.windowsMutex);
        app.windows.push_back(windowState);
    }
    m_windows.push_back(std::move(window));

    EnsureClientTitleBar(nativeHandle, NeoWPF::Contracts::ChromeMode::Native);

    return nativeHandle;
}

bool LWindowing::GetNativeWindowInfo(void* nativeWindowHandle,
                                     NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept
{
    outInfo = {};
    const auto* window = FindWindow(nativeWindowHandle);
    if (!window || !window->IsValid()) {
        return false;
    }
    outInfo.system = NeoWPF::Contracts::NativeWindowSystem::Wayland;
    outInfo.window = window->Surface();
    outInfo.display = window->Display();
    outInfo.vulkanInstanceExtensions = VulkanInstanceExtensions;
    outInfo.vulkanInstanceExtensionCount = std::size(VulkanInstanceExtensions);
    outInfo.createVulkanSurface = &CreateVulkanSurface;
    return true;
}

bool LWindowing::IsWindowAlive(void* nativeWindowHandle) const noexcept
{
    const auto* window = FindWindow(nativeWindowHandle);
    return window && window->IsValid() && !window->IsClosed();
}

void LWindowing::GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth,
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

void LWindowing::ResizeWindow(void* nativeWindowHandle, int width, int height) noexcept
{
    if (auto* window = FindWindow(nativeWindowHandle)) {
        window->SetSize(width, height);
        wl_surface_commit(window->Surface());
        wl_display_flush(window->Display());
    }
}

void LWindowing::ShowWindow(void* nativeWindowHandle) noexcept
{
    if (auto* window = FindWindow(nativeWindowHandle)) {
        wl_surface_commit(window->Surface());
        wl_display_flush(window->Display());
    }
}

void LWindowing::HideWindow(void*) noexcept
{
}

void LWindowing::CloseWindow(void* nativeWindowHandle) noexcept
{
    auto it = std::find_if(m_windows.begin(), m_windows.end(), [nativeWindowHandle](const auto& window) {
        return window.get() == nativeWindowHandle;
    });
    if (it == m_windows.end()) {
        return;
    }
    (*it)->Close();
    NeoWPF::Core::CloseApplicationWindow(nativeWindowHandle);
    m_windows.erase(it);
}

void LWindowing::InvalidateWindow(void* nativeWindowHandle) noexcept
{
    ShowWindow(nativeWindowHandle);
}

bool LWindowing::PumpMessages(std::chrono::milliseconds timeout)
{
    if (!m_display || wl_display_dispatch_pending(m_display) < 0) {
        return false;
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

int LWindowing::ExitCode() const noexcept
{
    return m_exitCode;
}

void LWindowing::RequestQuit(int exitCode) noexcept
{
    m_exitCode = exitCode;
    while (!m_windows.empty()) {
        CloseWindow(m_windows.back().get());
    }
}

void LWindowing::ShowFatalError(const std::wstring&, const std::wstring& message) noexcept
{
    NeoWPF::Diagnostics::LogError("%s", NeoWPF::Platform::WideToUtf8(message).c_str());
}

void LWindowing::SetConsoleVisible(bool visible) noexcept
{
    m_consoleVisible = visible;
}

bool LWindowing::IsConsoleVisible() const noexcept
{
    return m_consoleVisible;
}

LWindow* LWindowing::FindWindow(void* nativeWindowHandle) noexcept
{
    const auto it = std::find_if(m_windows.begin(), m_windows.end(), [nativeWindowHandle](const auto& window) {
        return window.get() == nativeWindowHandle;
    });
    return it == m_windows.end() ? nullptr : it->get();
}

const LWindow* LWindowing::FindWindow(void* nativeWindowHandle) const noexcept
{
    const auto it = std::find_if(m_windows.begin(), m_windows.end(), [nativeWindowHandle](const auto& window) {
        return window.get() == nativeWindowHandle;
    });
    return it == m_windows.end() ? nullptr : it->get();
}

void LWindowing::Disconnect() noexcept
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
    if (m_xdgWmBase) {
        xdg_wm_base_destroy(m_xdgWmBase);
        m_xdgWmBase = nullptr;
    }
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

bool LWindowing::IsConnected() const noexcept
{
    return m_display != nullptr && m_registry != nullptr;
}

wl_display* LWindowing::Display() const noexcept
{
    return m_display;
}

wl_compositor* LWindowing::Compositor() const noexcept
{
    return m_compositor;
}

wl_shm* LWindowing::Shm() const noexcept
{
    return m_shm;
}

wl_seat* LWindowing::Seat() const noexcept
{
    return m_seat;
}

xdg_wm_base* LWindowing::XdgWmBase() const noexcept
{
    return m_xdgWmBase;
}

bool LWindowing::HasXdgShell() const noexcept
{
    return m_hasXdgShell;
}

void LWindowing::HandleGlobal(void* data, wl_registry* registry, std::uint32_t name,
                              const char* interface, std::uint32_t version)
{
    auto* windowing = static_cast<LWindowing*>(data);
    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
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
    } else if (std::strcmp(interface, "xdg_wm_base") == 0) {
        windowing->m_xdgWmBase = static_cast<xdg_wm_base*>(wl_registry_bind(
            registry, name, &xdg_wm_base_interface, 1));
        if (windowing->m_xdgWmBase) {
            xdg_wm_base_add_listener(windowing->m_xdgWmBase, &XdgWmBaseListener, windowing);
            windowing->m_hasXdgShell = true;
        }
    }
}

void LWindowing::HandleGlobalRemove(void*, wl_registry*, std::uint32_t)
{
}

}