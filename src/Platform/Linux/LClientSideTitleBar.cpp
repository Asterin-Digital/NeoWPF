#include <NeoWPF/Platform/Linux/LClientSideTitleBar.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/UI/Controls/DefaultTitleBar.h>

#include <mutex>

namespace NeoWPF::Platform::Linux {

std::shared_ptr<NeoWPF::UI::Visual> CreateDefaultClientTitleBar(void* nativeHandle,
                                                                const std::wstring& title)
{
    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info,
        "[LinuxClientTitleBar] CreateDefaultClientTitleBar nativeHandle=%p title='%ls'",
        nativeHandle,
        title.empty() ? L"NeoWPF" : title.c_str());

    auto defaultTitleBar = std::make_shared<NeoWPF::UI::DefaultTitleBar>();
    defaultTitleBar->SetTitle(title.empty() ? L"NeoWPF" : title);

    defaultTitleBar->SetOnMinimizeClicked([nativeHandle]() {
        if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
            if (auto* chrome = platform->Chrome()) {
                chrome->SetMinimized(nativeHandle);
            }
        }
    });

    defaultTitleBar->SetOnMaximizeClicked([nativeHandle]() {
        if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
            if (auto* chrome = platform->Chrome()) {
                const bool maximized = chrome->IsMaximized(nativeHandle);
                chrome->SetMaximized(nativeHandle, !maximized);
            }
        }
    });

    defaultTitleBar->SetOnCloseClicked([nativeHandle]() {
        if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
            platform->Windowing().CloseWindow(nativeHandle);
        }
    });

    return defaultTitleBar;
}

void EnsureClientTitleBar(void* nativeHandle, NeoWPF::Contracts::ChromeMode mode)
{
    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info,
        "[LinuxClientTitleBar] EnsureClientTitleBar nativeHandle=%p mode=%d",
        nativeHandle,
        static_cast<int>(mode));

    if (!nativeHandle) {
        NeoWPF::Diagnostics::Log(
            NeoWPF::Diagnostics::Category::Application,
            NeoWPF::Diagnostics::Level::Info,
            "[LinuxClientTitleBar] EnsureClientTitleBar aborted: null nativeHandle");
        return;
    }

    auto& app = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(app.windowSessionsMutex);

    for (auto& windowSession : app.windowSessions) {
        if (windowSession.nativeWindowHandle != nativeHandle || !windowSession.windowHost) {
            continue;
        }

        if ((mode == NeoWPF::Contracts::ChromeMode::Native || mode == NeoWPF::Contracts::ChromeMode::Custom) &&
            !windowSession.windowHost->GetTitleBar()) {
            const auto* titleText = windowSession.windowHost->GetTitle();
            const std::wstring title = titleText ? std::wstring(titleText) : std::wstring(L"NeoWPF");
            NeoWPF::Diagnostics::Log(
                NeoWPF::Diagnostics::Category::Application,
                NeoWPF::Diagnostics::Level::Info,
                "[LinuxClientTitleBar] Installing client-side titlebar for window=%p title='%ls' mode=%d",
                nativeHandle,
                title.c_str(),
                static_cast<int>(mode));
            windowSession.windowHost->SetTitleBar(CreateDefaultClientTitleBar(nativeHandle, title));
            return;
        }

        if (mode == NeoWPF::Contracts::ChromeMode::None) {
            NeoWPF::Diagnostics::Log(
                NeoWPF::Diagnostics::Category::Application,
                NeoWPF::Diagnostics::Level::Info,
                "[LinuxClientTitleBar] Clearing client-side titlebar for window=%p mode=%d",
                nativeHandle,
                static_cast<int>(mode));
            windowSession.windowHost->SetTitleBar(nullptr);
        }
        return;
    }

    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info,
        "[LinuxClientTitleBar] No matching window found for nativeHandle=%p while ensuring titlebar",
        nativeHandle);
}

}
