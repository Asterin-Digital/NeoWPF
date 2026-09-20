#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/CrashHandler.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/Diagnostics/TextDumper.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Core/Threading/ThreadAffinity.h>
#include <NeoWPF/UI/Localization/LocalizationManager.h>

#include <NeoWPFBuildInfo.h>

#include <atomic>

#ifndef NEOWPF_SLOW_ENV
#define NEOWPF_SLOW_ENV 2
#endif

namespace NeoWPF::Core {

namespace {

std::atomic<bool> s_initialized{false};

}

std::shared_ptr<Application> Application::s_current = nullptr;

void Application::AddLocale(std::shared_ptr<NeoWPF::UI::Localization::LocaleTable> table)
{
    if (!table || !table->IsValid()) {
        return;
    }
    if (NeoWPF::UI::Localization::LocalizationManager::Get().ReplaceTable(*table)) {
        m_locales.push_back(std::move(table));
    }
}

void Application::Initialize()
{
    NEO_WPF_STARTUP_ACTION("Application.Initialize");
    NeoWPF::Core::Threading::ThreadAffinity::RegisterMainThread();
    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info,
        "NeoWPF\nVersion: %s\nBuilt at: %s\nConfiguration: %s",
        NEOWPF_VERSION_STRING,
        NEOWPF_BUILD_TIME,
        NEOWPF_BUILD_CONFIGURATION);
    if (!NeoWPF::Platform::HasPlatform()) {
        NeoWPF::Platform::RegisterPlatform(NeoWPF::Platform::CreateDefaultPlatform());
    }
    if (!NeoWPF::Platform::HasPlatform()) {
        NeoWPF::Platform::ExitForMissingPlatform();
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
        s_current = std::make_shared<Application>();
    }
    auto& appState = GetAppState();
    appState.mainThreadExecutor = std::make_unique<NeoWPF::Core::Threading::MainThreadExecutor>();
    appState.backgroundExecutor = std::make_unique<NeoWPF::Core::Threading::BackgroundExecutor>();
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    NeoWPF::UI::Markup::Xaml::ValidateXamlRegistrations();
    NeoWPF::UI::Markup::Xaml::RegisterXamlMarkupExtensions();
    s_initialized.store(true, std::memory_order_release);

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "Application::Initialize complete");
}

NeoWPF::Platform::Window Application::CreateWindow(const wchar_t* title, int width, int height)
{
    return CreateWindow(title, width, height, nullptr);
}

NeoWPF::Platform::Window Application::CreateWindow(
    const wchar_t* title,
    int width,
    int height,
    const std::shared_ptr<NeoWPF::UI::Window>& windowHost)
{
    if (!s_initialized.load(std::memory_order_acquire)) {
        NeoWPF::Platform::ExitForUninitializedApplication();
    }
    if (!title) {
        return NeoWPF::Platform::Window();
    }
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform) {
        return NeoWPF::Platform::Window();
    }
    NEO_WPF_STARTUP_ACTION("Application.CreateWindow");    NeoWPF::Diagnostics::PerformanceMetrics::RecordWindowCreate();
    void* nativeHandle = platform->Windowing().CreateWindow(title, width, height, windowHost);
    return NeoWPF::Platform::Window(nativeHandle);
}

NeoWPF::Platform::Window Application::CreateWindow(
    const std::shared_ptr<NeoWPF::UI::Window>& windowHost,
    int width,
    int height)
{
    if (!windowHost) {
        return NeoWPF::Platform::Window();
    }
    return CreateWindow(windowHost->GetTitle(), width, height, windowHost);
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
    if (!NeoWPF::Platform::HasPlatform()) {
        NeoWPF::Platform::ExitForMissingPlatform();
    }
    if (appState.windows.empty()) {
        NeoWPF::Platform::CurrentPlatform()->Windowing().ShowFatalError(
            L"NeoWPF Error", L"No windows created. Call CreateWindow() before Run().");
        return -1;
    }

    StartRenderThread();

    constexpr auto renderInitializationTimeout = std::chrono::seconds(NEOWPF_SLOW_ENV);
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
    auto& appState = GetAppState();
    if (appState.backgroundExecutor) {
        appState.backgroundExecutor->Stop();
        appState.backgroundExecutor.reset();
    }
    if (appState.mainThreadExecutor) {
        appState.mainThreadExecutor->Stop();
        appState.mainThreadExecutor.reset();
    }
    NeoWPF::Core::Threading::ThreadAffinity::ClearMainThread();
    s_initialized.store(false, std::memory_order_release);
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().Clear();
    NeoWPF::Core::ApplicationResources::Resources::Shutdown();
    NeoWPF::Diagnostics::PerformanceMetrics::Shutdown();
    ShutdownApplication();
}

void Application::SetResource(const std::string& key, std::any value)
{
    NeoWPF::Core::ApplicationResources::Resources::SetResource(key, std::move(value));
}

bool Application::SetXamlResource(const std::string& key, const std::any& value)
{
    if (key.empty()) {
        return false;
    }
    NeoWPF::Core::ApplicationResources::Resources::SetResource(key, value);
    return true;
}

bool Application::SetImplicitXamlStyle(
    const std::type_index& targetType,
    const std::shared_ptr<NeoWPF::UI::Styling::Style>& style)
{
    if (targetType == std::type_index(typeid(void)) || !style) {
        return false;
    }
    NeoWPF::Core::ApplicationResources::Resources::SetImplicitStyle(targetType, style);
    return true;
}

bool Application::TryFindResource(const std::string& key, std::any& outValue)
{
    return NeoWPF::Core::ApplicationResources::Resources::TryFindResource(key, outValue);
}

bool Application::TryFindXamlResource(const std::string& key, std::any& outValue) const
{
    return NeoWPF::Core::ApplicationResources::Resources::TryFindResource(key, outValue);
}

bool Application::TryFindImplicitStyle(
    const std::type_index& targetType,
    std::shared_ptr<NeoWPF::UI::Styling::Style>& outStyle) const
{
    return NeoWPF::Core::ApplicationResources::Resources::TryFindImplicitStyle(targetType, outStyle);
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
    if (!s_initialized.load(std::memory_order_acquire)) {
        NeoWPF::Platform::ExitForUninitializedApplication();
    }
    return *s_current;
}

void Application::SetCurrent(std::shared_ptr<Application> application)
{
    s_current = std::move(application);
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

