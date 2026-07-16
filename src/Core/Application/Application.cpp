#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Styling/Style/DefaultStyles.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>

namespace NeoWPF::Core {

Application* Application::s_current = nullptr;
bool Application::s_applicationDebugging = true;

void Application::Initialize()
{
    if (s_applicationDebugging) {
        NeoWPF::Diagnostics::LogInfo("Application::Initialize begin");
    }
    GetAppState().hInst = GetModuleHandleW(nullptr);
    if (!s_current) {
        s_current = new Application();
    }
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    NeoWPF::UI::Styling::RegisterDefaultStyles();
    NeoWPF::UI::Visual::CaptureRootVisuals();

    if (s_applicationDebugging) {
        NeoWPF::Diagnostics::LogInfo("Application::Initialize complete");
    }
}

NeoWPF::Platform::Window Application::CreateWindow(const wchar_t* title, int width, int height)
{
    if (s_applicationDebugging) {
        NeoWPF::Diagnostics::LogInfo("Application::CreateWindow title=%ls", title);
    }
    return NeoWPF::Platform::Window(CreateApplicationWindow(title, width, height));
}

bool Application::CloseWindow(const NeoWPF::Platform::Window& window)
{
    if (s_applicationDebugging) {
        NeoWPF::Diagnostics::LogInfo("Application::CloseWindow");
    }
    return CloseApplicationWindow(window.Handle());
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
    ShutdownApplication();
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

