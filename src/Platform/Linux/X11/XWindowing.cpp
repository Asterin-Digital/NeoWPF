#include <NeoWPF/Platform/Linux/X11/XWindowing.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/Linux/X11/XClipboard.h>
#include <NeoWPF/Platform/Linux/X11/XWindow.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#ifndef VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <poll.h>

namespace NeoWPF::Platform::Linux {
namespace {

const char* VulkanInstanceExtensions[] = {
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
};

bool CreateVulkanSurface(void*, void* instance, void* window, void* display,
                         void* outSurface) noexcept
{
    const VkXlibSurfaceCreateInfoKHR createInfo{
        VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        nullptr,
        0,
        static_cast<Display*>(display),
        static_cast<::Window>(reinterpret_cast<std::uintptr_t>(window)),
    };
    const VkResult result = vkCreateXlibSurfaceKHR(
        static_cast<VkInstance>(instance), &createInfo, nullptr,
        static_cast<VkSurfaceKHR*>(outSurface));
    NeoWPF::Diagnostics::LogInfo(
        "[Vulkan/X11] vkCreateXlibSurfaceKHR: instance=%p display=%p window=%p result=%d",
        instance, display, window, static_cast<int>(result));
    return result == VK_SUCCESS;
}

}

XWindowing::XWindowing() = default;

XWindowing::~XWindowing()
{
    Disconnect();
}

bool XWindowing::Connect()
{
    if (IsConnected()) {
        return true;
    }
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        NeoWPF::Diagnostics::Catastrophic("[X11] XOpenDisplay failed");
        return false;
    }
    m_screen = DefaultScreen(m_display);
    m_root = ScreenOfDisplay(m_display, m_screen)->root;
    m_wmProtocols = XInternAtom(m_display, "WM_PROTOCOLS", False);
    m_wmDeleteWindow = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
    return true;
}

void XWindowing::Disconnect() noexcept
{
    for (auto& window : m_windows) {
        window->Close();
    }
    m_windows.clear();
    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
    m_root = 0;
}

bool XWindowing::IsConnected() const noexcept
{
    return m_display != nullptr;
}

Display* XWindowing::DisplayHandle() const noexcept
{
    return m_display;
}

::Window XWindowing::RootHandle() const noexcept
{
    return m_root;
}

Atom XWindowing::WmProtocolsAtom() const noexcept
{
    return m_wmProtocols;
}

Atom XWindowing::WmDeleteWindowAtom() const noexcept
{
    return m_wmDeleteWindow;
}

void* XWindowing::CreateWindow(
    const wchar_t* title,
    int width,
    int height,
    std::shared_ptr<NeoWPF::UI::Window> windowHost)
{
    if (!Connect()) {
        return nullptr;
    }
    const bool transparent = windowHost && windowHost->IsTransparent();
    auto window = std::make_unique<XWindow>(*this, title, width, height, transparent);
    if (!window->IsValid()) {
        return nullptr;
    }

    void* nativeHandle = window->NativeHandle();
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
    return nativeHandle;
}

XWindow* XWindowing::FindWindow(void* nativeWindowHandle) noexcept
{
    const ::Window target = static_cast<::Window>(reinterpret_cast<std::uintptr_t>(nativeWindowHandle));
    const auto it = std::find_if(m_windows.begin(), m_windows.end(),
        [target](const auto& window) { return window->XWindowHandle() == target; });
    return it == m_windows.end() ? nullptr : it->get();
}

const XWindow* XWindowing::FindWindow(void* nativeWindowHandle) const noexcept
{
    const ::Window target = static_cast<::Window>(reinterpret_cast<std::uintptr_t>(nativeWindowHandle));
    const auto it = std::find_if(m_windows.begin(), m_windows.end(),
        [target](const auto& window) { return window->XWindowHandle() == target; });
    return it == m_windows.end() ? nullptr : it->get();
}

bool XWindowing::GetNativeWindowInfo(void* nativeWindowHandle,
                                     NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept
{
    outInfo = {};
    const XWindow* window = FindWindow(nativeWindowHandle);
    if (!window || !window->IsValid()) {
        return false;
    }
    outInfo.system = NeoWPF::Contracts::NativeWindowSystem::X11;
    outInfo.surfaceAlpha = NeoWPF::Contracts::SurfaceAlpha::Opaque;
    outInfo.window = window->NativeHandle();
    outInfo.display = window->DisplayHandle();
    outInfo.vulkanInstanceExtensions = VulkanInstanceExtensions;
    outInfo.vulkanInstanceExtensionCount = std::size(VulkanInstanceExtensions);
    outInfo.createVulkanSurface = &CreateVulkanSurface;
    return true;
}

bool XWindowing::IsWindowAlive(void* nativeWindowHandle) const noexcept
{
    const XWindow* window = FindWindow(nativeWindowHandle);
    return window && window->IsValid() && !window->IsClosed();
}

void XWindowing::GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth,
                               unsigned int& outHeight) const noexcept
{
    outWidth = 0;
    outHeight = 0;
    const XWindow* window = FindWindow(nativeWindowHandle);
    if (window) {
        outWidth = static_cast<unsigned int>(window->Width());
        outHeight = static_cast<unsigned int>(window->Height());
    }
}

bool XWindowing::GetWindowScaleFactor(void* nativeWindowHandle,
                                      double& outScaleFactor) const noexcept
{
    const XWindow* window = FindWindow(nativeWindowHandle);
    if (!window) {
        outScaleFactor = 0.0;
        return false;
    }
    outScaleFactor = window->ScaleFactor();
    return outScaleFactor > 0.0;
}

bool XWindowing::GetWindowInsets(void* nativeWindowHandle,
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

bool XWindowing::GetInputProfile(void* nativeWindowHandle,
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

void XWindowing::ResizeWindow(void* nativeWindowHandle, int width, int height) noexcept
{
    if (XWindow* window = FindWindow(nativeWindowHandle)) {
        window->Resize(width, height);
    }
}

void XWindowing::ShowWindow(void* nativeWindowHandle) noexcept
{
    if (XWindow* window = FindWindow(nativeWindowHandle)) {
        window->Show();
    }
}

void XWindowing::HideWindow(void* nativeWindowHandle) noexcept
{
    if (XWindow* window = FindWindow(nativeWindowHandle)) {
        window->Hide();
    }
}

void XWindowing::CloseWindow(void* nativeWindowHandle) noexcept
{
    auto it = std::find_if(m_windows.begin(), m_windows.end(),
        [nativeWindowHandle](const auto& window) {
            return window->NativeHandle() == nativeWindowHandle;
        });
    if (it == m_windows.end()) {
        return;
    }
    NeoWPF::Core::CloseWindow(nativeWindowHandle);
    (*it)->Close();
    m_windows.erase(it);
}

void XWindowing::InvalidateWindow(void* nativeWindowHandle) noexcept
{
    if (XWindow* window = FindWindow(nativeWindowHandle)) {
        window->Show();
    }
}

bool XWindowing::PumpMessages(std::chrono::milliseconds timeout)
{
    if (!m_display) {
        return false;
    }
    pollfd descriptor{ConnectionNumber(m_display), POLLIN, 0};
    const int pollResult = poll(&descriptor, 1, static_cast<int>(timeout.count()));
    if (pollResult < 0) {
        return false;
    }
    if (pollResult > 0) {
        while (XPending(m_display) > 0) {
            XEvent event{};
            XNextEvent(m_display, &event);
            if (event.type == SelectionRequest && m_clipboard) {
                m_clipboard->ProcessSelectionRequest(event);
            } else if (event.type == SelectionNotify && m_clipboard) {
                m_clipboard->ProcessSelectionNotify(event);
            } else {
                XWindow* window = FindWindow(reinterpret_cast<void*>(
                    static_cast<std::uintptr_t>(event.xany.window)));
                if (window) {
                    window->HandleEvent(event);
                }
            }
        }
    }
    XFlush(m_display);
    for (auto& window : m_windows) {
        window->ProcessKeyRepeat();
        if (window->IsClosed()) {
            CloseWindow(window->NativeHandle());
            return !m_windows.empty() && m_exitCode == 0;
        }
    }
    return !m_windows.empty() && m_exitCode == 0;
}

int XWindowing::ExitCode() const noexcept
{
    return m_exitCode;
}

void XWindowing::RequestQuit(int exitCode) noexcept
{
    m_exitCode = exitCode;
    while (!m_windows.empty()) {
        CloseWindow(m_windows.back()->NativeHandle());
    }
}

void XWindowing::ShowFatalError(const std::wstring&, const std::wstring& message) noexcept
{
    const auto converted = NeoWPF::Platform::WideToUtf8(message);
    NeoWPF::Diagnostics::Catastrophic(
        "%s", converted.Success() ? converted.Value().c_str() : converted.Error().c_str());
}

void XWindowing::SetConsoleVisible(bool visible) noexcept
{
    m_consoleVisible = visible;
}

bool XWindowing::IsConsoleVisible() const noexcept
{
    return m_consoleVisible;
}

void XWindowing::AttachClipboard(XClipboard* clipboard) noexcept
{
    m_clipboard = clipboard;
}

void XWindowing::DetachClipboard(XClipboard* clipboard) noexcept
{
    if (m_clipboard == clipboard) {
        m_clipboard = nullptr;
    }
}

}
