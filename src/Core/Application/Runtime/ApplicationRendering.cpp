#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Diagnostics/TextDumper.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Controls/Window.h>

#include <chrono>
#include <mutex>

namespace NeoWPF::Core {

int RunDesktopApp(std::chrono::seconds renderInitializationTimeout)
{
    auto& appState = GetAppState();
    StartRenderThread();
    std::unique_lock<std::mutex> lock(appState.renderInitMutex);
    if (!appState.renderInitCondition.wait_for(lock, renderInitializationTimeout, [] {
            return GetAppState().renderThreadInitialized.load() || GetAppState().renderThreadInitFailed.load();
        })) {
        NeoWPF::Platform::CurrentPlatform()->Windowing().ShowFatalError(
            L"NeoWPF Error", L"NeoWPF render initialization timed out.");
        appState.running = false;
        if (appState.renderThread.joinable()) {
            appState.renderThread.join();
        }
        return -1;
    }

    if (appState.renderThreadInitFailed.load()) {
        NeoWPF::Platform::CurrentPlatform()->Windowing().ShowFatalError(
            L"NeoWPF Error", L"NeoWPF failed to initialize rendering.");
        appState.running = false;
        if (appState.renderThread.joinable()) {
            appState.renderThread.join();
        }
        return -1;
    }

    return RunMessageLoop();
}

NeoWPF::Result<void> Application::LoadFontFromFile(const std::wstring& filePath)
{
    NEO_WPF_STARTUP_ACTION("Application.LoadFontFromFile");
    if (filePath.empty()) {
        NeoWPF::Diagnostics::LogError("Application::LoadFontFromFile: empty file path supplied");
        return NeoWPF::Fail<std::string>{"Font file path cannot be empty"};
    }

    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info,
        "Application::LoadFontFromFile: attempting to load custom font '%ls'",
        filePath.c_str());

    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform) {
        return NeoWPF::Fail<std::string>{"No platform is registered for font loading"};
    }

    return platform->RegisterFontFile(filePath);
}

NeoWPF::Result<void> Application::LoadFontFromMemory(
    const unsigned char* data,
    size_t size,
    const std::wstring& familyName)
{
    NEO_WPF_STARTUP_ACTION("Application.LoadFontFromMemory");
    if (!data || size == 0 || familyName.empty()) {
        NeoWPF::Diagnostics::LogError("Application::LoadFontFromMemory: invalid font data or family name");
        return NeoWPF::Fail<std::string>{"Font data and family name must be provided"};
    }

    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info,
        "Application::LoadFontFromMemory: registering '%ls' (%zu bytes)",
        familyName.c_str(), size);

    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform) {
        return NeoWPF::Fail<std::string>{"No platform is registered for font loading"};
    }
    return platform->RegisterFontMemory(data, size, familyName);
}

void Application::SetLayoutInspection(bool enabled)
{
    GetAppState().layoutDebugLogging.store(enabled);
}

bool Application::IsLayoutInspectionEnabled()
{
    return GetAppState().layoutDebugLogging.load();
}

void Application::DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options)
{
    DumpTree(options);
}

void Application::DumpLayoutTree()
{
    NeoWPF::Diagnostics::VisualTreeDumpOptions options;
    options.flags = NeoWPF::Diagnostics::IncludeLayout | NeoWPF::Diagnostics::IncludeProperties;
    options.maxDepth = 0;
    DumpTree(options);
}

void Application::DumpTextTree(const NeoWPF::Diagnostics::TextDumpOptions& options)
{
    auto& appState = GetAppState();
    if (appState.windowSessions.empty()) {
        NeoWPF::Diagnostics::LogWarn("No text tree to dump, no windows have content attached");
        return;
    }

    for (const auto& windowSession : appState.windowSessions) {
        if (windowSession.windowHost && windowSession.windowHost->GetContent()) {
            auto rootVisual = windowSession.windowHost->GetContent();
            rootVisual->Initialize();
            NeoWPF::Diagnostics::TextDumper::Dump(rootVisual.get(), options);
        }
    }
}

}
