#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Root/RootVisuals.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/Window.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Visual.h>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Styling/DefaultStyles.h>
#include <NeoWPF/Input/InputManager.h>
#include <exception>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <utility>
#include <vector>
#include <map>

namespace NeoWPF::Core {

struct WindowState {
    HWND hwnd = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    bool needsLayout = true;
};

struct AppState {
    HINSTANCE hInst = nullptr;
    std::vector<WindowState> windows;
    std::atomic<bool> running{false};
    std::atomic<bool> renderThreadInitialized{false};
    std::atomic<bool> renderThreadInitFailed{false};
    std::thread renderThread;
    int exitCode = 0;

    std::mutex renderInitMutex;
    std::condition_variable renderInitCondition;

    std::mutex windowsMutex;
    std::mutex commandListMutex;
    NeoWPF::Rendering::CommandList renderCommandList;
    NeoWPF::Rendering::CommandList buildCommandList;
    bool commandListInitialized = false;
};

static AppState s_appState;
Application* Application::s_current = nullptr;

void StartRenderThread()
{
    s_appState.running = true;
    s_appState.renderThreadInitialized = false;
    s_appState.renderThreadInitFailed = false;
    s_appState.renderThread = std::thread([]() {
        try {
            // Initialize renderer for all windows
            {
                std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
                for (const auto& windowState : s_appState.windows) {
                    NeoWPF::Rendering::Renderer_Initialize(windowState.hwnd);
                }
            }
            {
                std::lock_guard<std::mutex> lock(s_appState.renderInitMutex);
                s_appState.renderThreadInitialized = true;
            }
            s_appState.renderInitCondition.notify_one();
        }
        catch (const std::exception& ex) {
            NeoWPF::Diagnostics::LogError("Render thread initialization failed: %s", ex.what());
            {
                std::lock_guard<std::mutex> lock(s_appState.renderInitMutex);
                s_appState.renderThreadInitFailed = true;
            }
            s_appState.renderInitCondition.notify_one();
            s_appState.running = false;
            return;
        }
        catch (...) {
            NeoWPF::Diagnostics::LogError("Render thread initialization failed with unknown error");
            {
                std::lock_guard<std::mutex> lock(s_appState.renderInitMutex);
                s_appState.renderThreadInitFailed = true;
            }
            s_appState.renderInitCondition.notify_one();
            s_appState.running = false;
            return;
        }

        // Render loop
        while (s_appState.running.load()) {
            std::vector<WindowState> windowsSnapshot;
            {
                std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
                windowsSnapshot = s_appState.windows;
            }

            // Render each window
            for (const auto& windowState : windowsSnapshot) {
                RECT rc;
                GetClientRect(windowState.hwnd, &rc);
                unsigned int w = rc.right - rc.left;
                unsigned int h = rc.bottom - rc.top;
                
                if (w > 0 && h > 0) {
                    NeoWPF::Rendering::Renderer_Resize(w, h);
                    NeoWPF::Rendering::Renderer_BeginFrame();

                    NeoWPF::Rendering::CommandList toRender;
                    {
                        std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
                        toRender = s_appState.renderCommandList;
                    }

                    NeoWPF::Rendering::Renderer_Execute(toRender);
                    NeoWPF::Rendering::Renderer_Present();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        // Shutdown renderer
        {
            std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
            for (const auto& windowState : s_appState.windows) {
                NeoWPF::Rendering::Renderer_Shutdown();
            }
        }
    });
}

void Application::Initialize()
{
    s_appState.hInst = GetModuleHandleW(nullptr);
    if (!s_current) {
        s_current = new Application();
    }
    NeoWPF::UI::Styling::RegisterDefaultStyles();

    NeoWPF::UI::Visual::CaptureRootVisuals();
}

HWND Application::CreateWindow(const wchar_t* title, int width, int height)
{
    if (!s_appState.hInst) {
        s_appState.hInst = GetModuleHandleW(nullptr);
    }

    HWND hwnd = NeoWPF::Platform::Window::CreateWindow(s_appState.hInst, title, width, height);
    if (!hwnd) {
        return nullptr;
    }

    WindowState windowState;
    windowState.hwnd = hwnd;
    windowState.width = width;
    windowState.height = height;
    windowState.needsLayout = true;

    {
        std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
        s_appState.windows.push_back(windowState);
    }

    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);

    return hwnd;
}

int Application::Run()
{
    if (s_appState.windows.empty()) {
        MessageBoxW(nullptr, L"No windows created. Call CreateWindow() before Run().", L"NeoWPF Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    StartRenderThread();

    std::unique_lock<std::mutex> lock(s_appState.renderInitMutex);
    if (!s_appState.renderInitCondition.wait_for(lock, std::chrono::seconds(2), [] {
            return s_appState.renderThreadInitialized.load() || s_appState.renderThreadInitFailed.load();
        })) {
        MessageBoxW(nullptr, L"NeoWPF render initialization timed out.", L"NeoWPF Error", MB_OK | MB_ICONERROR);
        s_appState.running = false;
        if (s_appState.renderThread.joinable()) {
            s_appState.renderThread.join();
        }
        return -1;
    }

    if (s_appState.renderThreadInitFailed.load()) {
        MessageBoxW(nullptr, L"NeoWPF failed to initialize rendering.", L"NeoWPF Error", MB_OK | MB_ICONERROR);
        s_appState.running = false;
        if (s_appState.renderThread.joinable()) {
            s_appState.renderThread.join();
        }
        return -1;
    }

    // Get the primary window for layout
    HWND primaryWindow = nullptr;
    {
        std::lock_guard<std::mutex> wlock(s_appState.windowsMutex);
        if (!s_appState.windows.empty()) {
            primaryWindow = s_appState.windows[0].hwnd;
        }
    }

    if (!primaryWindow) {
        MessageBoxW(nullptr, L"No primary window available.", L"NeoWPF Error", MB_OK | MB_ICONERROR);
        s_appState.running = false;
        return -1;
    }

    // Initial layout
    RECT rc;
    GetClientRect(primaryWindow, &rc);
    unsigned int clientWidth = rc.right - rc.left;
    unsigned int clientHeight = rc.bottom - rc.top;

    const auto& visuals = NeoWPF::UI::Visual::GetRegisteredVisuals();
    {
        std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
        s_appState.buildCommandList.Clear();
        LayoutAndBuildAllTopLevelVisuals(visuals, clientWidth, clientHeight, s_appState.buildCommandList);
        std::swap(s_appState.buildCommandList, s_appState.renderCommandList);
        s_appState.commandListInitialized = true;
    }
    ClearTopLevelVisualsDirty(visuals);

    const auto& rootInfos = FindRootVisual(visuals);
    auto rootVisual = rootInfos.rootVisual;
    NeoWPF::Input::InputManager::GetInstance().SetRootElement(dynamic_cast<NeoWPF::UI::UIElement*>(rootVisual));

    // Message loop
    MSG msg;
    unsigned int lastLayoutWidth = 0, lastLayoutHeight = 0;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // Check if any windows are still open
        {
            std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
            bool anyWindowOpen = false;
            for (const auto& window : s_appState.windows) {
                if (IsWindow(window.hwnd)) {
                    anyWindowOpen = true;
                    break;
                }
            }
            if (!anyWindowOpen) {
                break;
            }
        }

        bool needsUpdate = !s_appState.commandListInitialized;
        const auto& visuals = NeoWPF::UI::Visual::GetRegisteredVisuals();
        for (auto visual : visuals) {
            if (!visual->GetParent() && visual->IsTreeDirty()) {
                needsUpdate = true;
                break;
            }
        }

        RECT rc;
        GetClientRect(primaryWindow, &rc);
        unsigned int currentClientWidth = rc.right - rc.left;
        unsigned int currentClientHeight = rc.bottom - rc.top;
        if (currentClientWidth != lastLayoutWidth || currentClientHeight != lastLayoutHeight) {
            needsUpdate = true;
            lastLayoutWidth = currentClientWidth;
            lastLayoutHeight = currentClientHeight;
        }

        if (needsUpdate) {
            std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
            s_appState.buildCommandList.Clear();
            LayoutAndBuildAllTopLevelVisuals(visuals, currentClientWidth, currentClientHeight, s_appState.buildCommandList);
            std::swap(s_appState.buildCommandList, s_appState.renderCommandList);
            s_appState.commandListInitialized = true;
            ClearTopLevelVisualsDirty(visuals);
        }
    }

    s_appState.exitCode = static_cast<int>(msg.wParam);
    s_appState.running = false;
    if (s_appState.renderThread.joinable()) {
        s_appState.renderThread.join();
    }

    // Clean up windows
    {
        std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
        s_appState.windows.clear();
    }

    return s_appState.exitCode;
}

void Application::Shutdown()
{
    s_appState.running = false;
    PostQuitMessage(0);
}

Application* Application::Current()
{
    return s_current;
}

}
