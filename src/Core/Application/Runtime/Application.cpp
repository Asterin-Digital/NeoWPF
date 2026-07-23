#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>

namespace NeoWPF::Core {

Application* Application::s_current = nullptr;
bool Application::s_applicationDebugging = true;

void Application::Initialize()
{
    NeoWPF::Core::ApplicationResources::Resources::Initialize();
    NeoWPF::Diagnostics::PerformanceMetrics::Initialize();
#if defined(_DEBUG)
    ShowConsole(true);
#else
    ShowConsole(false);
#endif
    if (s_applicationDebugging) {
        NeoWPF::Diagnostics::LogInfo("Application::Initialize begin");
    }
    GetAppState().hInst = GetModuleHandleW(nullptr);
    if (!s_current) {
        s_current = new Application();
    }
    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    if (s_applicationDebugging) {
        NeoWPF::Diagnostics::LogInfo("Application::Initialize complete");
    }
}

NeoWPF::Platform::Window Application::CreateWindow(const wchar_t* title, int width, int height)
{
    if (!title) {
        return NeoWPF::Platform::Window();
    }

    NeoWPF::Diagnostics::PerformanceMetrics::RecordWindowCreate();
    const HWND hwnd = CreateApplicationWindow(title, width, height);
    return NeoWPF::Platform::Window(hwnd);
}

void Application::ShowConsole(bool visible)
{
    auto& appState = GetAppState();
    if (appState.consoleVisible.load() == visible) {
        return;
    }

    appState.consoleVisible.store(visible);

    if (visible) {
        if (AllocConsole()) {
            FILE* dummy = nullptr;
            freopen_s(&dummy, "CONOUT$", "w", stdout);
            freopen_s(&dummy, "CONOUT$", "w", stderr);
        }
    } else {
        FreeConsole();
    }
}

bool Application::IsConsoleVisible()
{
    return GetAppState().consoleVisible.load();
}

int Application::Run()
{
    auto& appState = GetAppState();
    if (appState.windows.empty()) {
        MessageBoxW(nullptr, L"No windows created. Call CreateWindow() before Run().", L"NeoWPF Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    StartRenderThread();

    std::unique_lock<std::mutex> lock(appState.renderInitMutex);
    if (!appState.renderInitCondition.wait_for(lock, std::chrono::seconds(2), [] {
            return GetAppState().renderThreadInitialized.load() || GetAppState().renderThreadInitFailed.load();
        })) {
        MessageBoxW(nullptr, L"NeoWPF render initialization timed out.", L"NeoWPF Error", MB_OK | MB_ICONERROR);
        appState.running = false;
        if (appState.renderThread.joinable()) {
            appState.renderThread.join();
        }
        return -1;
    }

    if (appState.renderThreadInitFailed.load()) {
        MessageBoxW(nullptr, L"NeoWPF failed to initialize rendering.", L"NeoWPF Error", MB_OK | MB_ICONERROR);
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

Application* Application::Current()
{
    return s_current;
}

void Application::DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options)
{
    DumpVisualTreeImpl(options);
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

