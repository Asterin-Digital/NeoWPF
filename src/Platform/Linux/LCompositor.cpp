#include <NeoWPF/Platform/Linux/LCompositor.h>

#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Platform/Linux/Wayland/WlClipboard.h>
#include <NeoWPF/Platform/Linux/Wayland/WlWindow.h>
#include <NeoWPF/Platform/Linux/Wayland/WlWindowChrome.h>
#include <NeoWPF/Platform/Linux/Wayland/WlWindowing.h>
#include <NeoWPF/Platform/Linux/X11/XClipboard.h>
#include <NeoWPF/Platform/Linux/X11/XWindow.h>
#include <NeoWPF/Platform/Linux/X11/XWindowChrome.h>
#include <NeoWPF/Platform/Linux/X11/XWindowing.h>

#undef Success
#undef None

#include <cstdlib>
#include <cstring>

namespace NeoWPF::Platform::Linux {
namespace {

class UnavailableWindowing final : public NeoWPF::Contracts::IWindowing {
public:
    void* CreateWindow(const wchar_t*, int, int,
        std::shared_ptr<NeoWPF::UI::Window> = nullptr) override
    {
        NeoWPF::Diagnostics::LogError(
            "Linux display backend unavailable. Set NEOWPF_LINUX_BACKEND=wayland or NEOWPF_LINUX_BACKEND=x11.");
        return nullptr;
    }
    bool GetNativeWindowInfo(void*, NeoWPF::Contracts::NativeWindowInfo&) const noexcept override { return false; }
    bool IsWindowAlive(void*) const noexcept override { return false; }
    void GetWindowSize(void*, unsigned int& width, unsigned int& height) const noexcept override
    {
        width = 0;
        height = 0;
    }
    bool GetWindowScaleFactor(void*, double& scale) const noexcept override
    {
        scale = 0.0;
        return false;
    }
    void ResizeWindow(void*, int, int) noexcept override {}
    void ShowWindow(void*) noexcept override {}
    void HideWindow(void*) noexcept override {}
    void CloseWindow(void*) noexcept override {}
    void InvalidateWindow(void*) noexcept override {}
    bool PumpMessages(std::chrono::milliseconds) override { return false; }
    int ExitCode() const noexcept override { return 1; }
    void RequestQuit(int) noexcept override {}
    void ShowFatalError(const std::wstring&, const std::wstring& message) noexcept override
    {
        NeoWPF::Diagnostics::LogError("%s", NeoWPF::Platform::WideToUtf8(message).c_str());
    }
    void SetConsoleVisible(bool) noexcept override {}
    bool IsConsoleVisible() const noexcept override { return false; }
};

class UnavailableClipboard final : public NeoWPF::Contracts::IClipboard {
public:
    NeoWPF::Result<void> SetText(const std::wstring&) override
    {
        return NeoWPF::Fail<std::string>{"Linux clipboard backend is unavailable"};
    }
    void TryGetText(TextCallback callback) override
    {
        callback(NeoWPF::Fail<std::string>{"Linux clipboard backend is unavailable"});
    }
    NeoWPF::Result<void> Clear() override
    {
        return NeoWPF::Fail<std::string>{"Linux clipboard backend is unavailable"};
    }
};

class UnavailableChrome final : public NeoWPF::Contracts::IWindowChrome {
public:
    NeoWPF::Contracts::ChromeMode DefaultChromeMode() const noexcept override
    {
        return NeoWPF::Contracts::ChromeMode::Native;
    }
    bool SupportsMode(NeoWPF::Contracts::ChromeMode) const noexcept override { return true; }
    bool SetChromeMode(void*, NeoWPF::Contracts::ChromeMode) noexcept override { return false; }
    NeoWPF::Contracts::ChromeMode GetChromeMode(void*) const noexcept override
    {
        return DefaultChromeMode();
    }
    void SetTitleBarHitTest(void*, NeoWPF::Contracts::ChromeHitTestCallback, void*) noexcept override {}
    void BeginInteractiveMove(void*) noexcept override {}
    void BeginInteractiveResize(void*, NeoWPF::Contracts::ResizeEdge) noexcept override {}
    void SetMaximized(void*, bool) noexcept override {}
    bool IsZoomed(void*) const noexcept override { return false; }
    bool IsMaximized(void*) const noexcept override { return false; }
    void SetMinimized(void*) noexcept override {}
    bool SetNativeCaptionColor(void*, std::uint32_t) noexcept override { return false; }
};

}

LinuxDisplayBackend ResolveLinuxDisplayBackend(
    const char* overrideValue,
    const char* xdgSessionType,
    const char* waylandDisplay) noexcept
{
    if (overrideValue) {
        if (std::strcmp(overrideValue, "wayland") == 0) {
            return LinuxDisplayBackend::Wayland;
        }
        if (std::strcmp(overrideValue, "x11") == 0) {
            return LinuxDisplayBackend::X11;
        }
    }
    if (xdgSessionType && std::strcmp(xdgSessionType, "x11") == 0) {
        return LinuxDisplayBackend::X11;
    }
    if (xdgSessionType && std::strcmp(xdgSessionType, "wayland") == 0) {
        return LinuxDisplayBackend::Wayland;
    }
    if (waylandDisplay) {
        return LinuxDisplayBackend::Wayland;
    }
    return LinuxDisplayBackend::X11;
}

LCompositor::LCompositor()
{
    m_windowing = std::make_unique<UnavailableWindowing>();
    m_clipboard = std::make_unique<UnavailableClipboard>();
    m_chrome = std::make_unique<UnavailableChrome>();

    const LinuxDisplayBackend choice = ResolveLinuxDisplayBackend(
        std::getenv("NEOWPF_LINUX_BACKEND"),
        std::getenv("XDG_SESSION_TYPE"),
        std::getenv("WAYLAND_DISPLAY"));
    if (choice == LinuxDisplayBackend::Wayland) {
        auto windowing = std::make_unique<WlWindowing>();
        if (windowing->Connect()) {
            auto clipboard = std::make_unique<WlClipboard>(*windowing);
            m_clipboard = std::move(clipboard);
            m_chrome = std::make_unique<WlWindowChrome>();
            m_windowing = std::move(windowing);
            m_connected = true;
        } else {
            NeoWPF::Diagnostics::LogError(
                "[Wayland] Selected Linux display backend could not connect");
        }
    } else {
        auto windowing = std::make_unique<XWindowing>();
        if (windowing->Connect()) {
            auto clipboard = std::make_unique<XClipboard>(*windowing);
            m_clipboard = std::move(clipboard);
            m_chrome = std::make_unique<XWindowChrome>(*windowing);
            m_windowing = std::move(windowing);
            m_connected = true;
        } else {
            NeoWPF::Diagnostics::LogError(
                "[X11] Selected Linux display backend could not connect");
        }
    }
}

LCompositor::~LCompositor() = default;

bool LCompositor::IsConnected() const noexcept
{
    return m_connected;
}

NeoWPF::Contracts::IWindowing& LCompositor::Windowing() noexcept
{
    return *m_windowing;
}

NeoWPF::Contracts::IClipboard& LCompositor::Clipboard() noexcept
{
    return *m_clipboard;
}

NeoWPF::Contracts::IWindowChrome* LCompositor::Chrome() noexcept
{
    return m_chrome.get();
}

}
