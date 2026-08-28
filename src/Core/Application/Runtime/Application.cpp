#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/CrashHandler.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/Diagnostics/TextDumper.h>
#include <NeoWPF/Platform/PlatformRegistry.h>

namespace NeoWPF::Core {

Application* Application::s_current = nullptr;

void Application::Initialize()
{
    NEO_WPF_STARTUP_ACTION("Application.Initialize");
    if (!NeoWPF::Platform::HasPlatform()) {
        NeoWPF::Platform::RegisterPlatform(NeoWPF::Platform::CreateDefaultPlatform());
    }
    if (!NeoWPF::Platform::HasPlatform()) {
        NeoWPF::Diagnostics::LogError("Application::Initialize: no platform backend is available");
        return;
    }
    NeoWPF::Diagnostics::InitializeCrashHandler();
    NeoWPF::Core::ApplicationResources::Resources::Initialize();
    NeoWPF::Diagnostics::PerformanceMetrics::Initialize();
#if defined(_DEBUG)
    ShowConsole(true);
#else
    ShowConsole(false);
#endif
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "Application::Initialize begin");
    if (!s_current) {
        s_current = new Application();
    }
    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "Application::Initialize complete");
}

NeoWPF::Platform::Window Application::CreateWindow(const wchar_t* title, int width, int height)
{
    if (!title) {
        return NeoWPF::Platform::Window();
    }
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform) {
        return NeoWPF::Platform::Window();
    }
    NEO_WPF_STARTUP_ACTION("Application.CreateWindow");    NeoWPF::Diagnostics::PerformanceMetrics::RecordWindowCreate();
    void* nativeHandle = platform->Windowing().CreateWindow(title, width, height);
    return NeoWPF::Platform::Window(nativeHandle);
}

void Application::ShowConsole(bool visible)
{
    NEO_WPF_STARTUP_ACTION("Application.ShowConsole");
    auto& appState = GetAppState();
    if (appState.consoleVisible.load() == visible) {
        return;
    }

    appState.consoleVisible.store(visible);

    if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
        platform->Windowing().SetConsoleVisible(visible);
    }
}

bool Application::IsConsoleVisible()
{
    if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
        return platform->Windowing().IsConsoleVisible();
    }
    return GetAppState().consoleVisible.load();
}

int Application::Run()
{
    auto& appState = GetAppState();
    if (appState.windows.empty()) {
        NeoWPF::Platform::CurrentPlatform()->Windowing().ShowFatalError(
            L"NeoWPF Error", L"No windows created. Call CreateWindow() before Run().");
        return -1;
    }

    StartRenderThread();

    constexpr auto renderInitializationTimeout = std::chrono::seconds(2);
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

    return RunApplicationMessageLoop();
}

void Application::Shutdown()
{
    NeoWPF::Core::ApplicationResources::Resources::Shutdown();
    NeoWPF::Diagnostics::PerformanceMetrics::Shutdown();
    ShutdownApplication();
}

void Application::SetResource(const std::string& key, std::any value)
{
    NeoWPF::Core::ApplicationResources::Resources::SetResource(key, std::move(value));
}

bool Application::TryFindResource(const std::string& key, std::any& outValue)
{
    return NeoWPF::Core::ApplicationResources::Resources::TryFindResource(key, outValue);
}

std::any Application::FindResource(const std::string& key)
{
    return NeoWPF::Core::ApplicationResources::Resources::FindResource(key);
}

bool Application::TryGetResource(const std::string& key, std::any& outValue)
{
    return NeoWPF::Core::ApplicationResources::Resources::TryGetResource(key, outValue);
}

NeoWPF::ResourceManager& Application::Resources()
{
    return NeoWPF::ResourceManager::Instance();
}

bool Application::LoadFontFromFile(const std::wstring& filePath)
{
    NEO_WPF_STARTUP_ACTION("Application.LoadFontFromFile");
    if (filePath.empty()) {
        NeoWPF::Diagnostics::LogError("Application::LoadFontFromFile: empty file path supplied");
        return false;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "Application::LoadFontFromFile: attempting to load custom font '%ls'", filePath.c_str());

    const bool registered = NeoWPF::Platform::CurrentPlatform()->RegisterFontFile(filePath);
    if (!registered) {
        NeoWPF::Diagnostics::LogError("Application::LoadFontFromFile: RegisterFontFile failed for '%ls'", filePath.c_str());
    }

    return registered;
}

bool Application::LoadFontFromMemory(
    const unsigned char* data,
    size_t size,
    const std::wstring& familyName)
{
    NEO_WPF_STARTUP_ACTION("Application.LoadFontFromMemory");
    if (!data || size == 0 || familyName.empty()) {
        NeoWPF::Diagnostics::LogError("Application::LoadFontFromMemory: invalid font data or family name");
        return false;
    }

    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info,
        "Application::LoadFontFromMemory: registering '%ls' (%zu bytes)",
        familyName.c_str(), size);

    const bool registered = NeoWPF::Platform::CurrentPlatform()->RegisterFontMemory(data, size, familyName);
    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info,
        "Application::LoadFontFromMemory: '%ls' registration %s",
        familyName.c_str(), registered ? "succeeded" : "failed");
    return registered;
}

Application& Application::Current()
{
    if (!s_current) {
        NeoWPF::Diagnostics::LogError("Application::Current: Application not initialized. Call Initialize() first.");
        std::abort();
    }
    return *s_current;
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
    DumpVisualTreeImpl(options);
}

void Application::DumpLayoutTree()
{
    NeoWPF::Diagnostics::VisualTreeDumpOptions options;
    options.flags = NeoWPF::Diagnostics::IncludeLayout | NeoWPF::Diagnostics::IncludeProperties;
    options.maxDepth = 0;
    DumpVisualTreeImpl(options);
}

void Application::DumpTextTree(const NeoWPF::Diagnostics::TextDumpOptions& options)
{
    auto& appState = GetAppState();
    if (appState.windows.empty()) {
        NeoWPF::Diagnostics::LogWarn("No text tree to dump, no windows have content attached");
        return;
    }

    for (const auto& windowState : appState.windows) {
        if (windowState.windowHost && windowState.windowHost->GetContent()) {
            auto rootVisual = windowState.windowHost->GetContent();
            rootVisual->Initialize();
            NeoWPF::Diagnostics::TextDumper::Dump(rootVisual.get(), options);
        }
    }
}

void Application::ShowDeveloperMessage(const std::wstring& title, const std::wstring& message, DeveloperMessageSeverity severity)
{
    NeoWPF::Diagnostics::QueueDeveloperWarning(title, message, severity);
}

void Application::ClearDeveloperMessages()
{
    NeoWPF::Diagnostics::ClearDeveloperWarnings();
}

}

