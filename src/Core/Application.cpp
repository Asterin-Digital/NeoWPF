#include <NeoWPF/Core/Application.h>
#include <NeoWPF/Core/Root/RootVisuals.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/Window.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Visual.h>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Styling/DefaultStyles.h>
#include <exception>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <utility>

namespace NeoWPF::Core {

struct AppState {
    HINSTANCE hInst = nullptr;
    HWND hwnd = nullptr;
    std::atomic<bool> running{false};
    std::atomic<bool> renderThreadInitialized{false};
    std::atomic<bool> renderThreadInitFailed{false};
    std::thread renderThread;
    int exitCode = 0;

    std::mutex renderInitMutex;
    std::condition_variable renderInitCondition;

    std::mutex commandListMutex;
    NeoWPF::Rendering::CommandList renderCommandList;
    NeoWPF::Rendering::CommandList buildCommandList;
    bool commandListInitialized = false;

    unsigned int lastClientWidth = 0;
    unsigned int lastClientHeight = 0;
};

static AppState s_appState;

void StartRenderThread(HWND hwnd)
{
    s_appState.running = true;
    s_appState.renderThreadInitialized = false;
    s_appState.renderThreadInitFailed = false;
    s_appState.renderThread = std::thread([hwnd]() {
        try {
            NeoWPF::Rendering::Renderer_Initialize(hwnd);
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

        while (s_appState.running.load()) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            unsigned int w = rc.right - rc.left;
            unsigned int h = rc.bottom - rc.top;
            if (w != s_appState.lastClientWidth || h != s_appState.lastClientHeight) {
                NeoWPF::Rendering::Renderer_Resize(w, h);
                s_appState.lastClientWidth = w;
                s_appState.lastClientHeight = h;
            }
            NeoWPF::Rendering::Renderer_BeginFrame();

            NeoWPF::Rendering::CommandList toRender;
            {
                std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
                toRender = s_appState.renderCommandList;
            }

            NeoWPF::Rendering::Renderer_Execute(toRender);
            NeoWPF::Rendering::Renderer_Present();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        NeoWPF::Rendering::Renderer_Shutdown();
    });
}

void Application::Initialize()
{
    s_appState.hInst = GetModuleHandleW(nullptr);
    NeoWPF::UI::Styling::RegisterDefaultStyles();
}

int Application::Run(const wchar_t* title, int width, int height)
{
    if (!s_appState.hInst) {
        s_appState.hInst = GetModuleHandleW(nullptr);
    }
    const auto& visuals = NeoWPF::UI::Visual::GetRegisteredVisuals();
    const auto rootInfo = FindRootVisual(visuals);
    auto rootVisual = rootInfo.rootVisual;
    if (rootInfo.rootWindow) {
        title = rootInfo.rootWindow->GetTitle();
    }

    s_appState.hwnd = NeoWPF::Platform::Window::CreateWindow(s_appState.hInst, title, width, height);
    if (!s_appState.hwnd) {
        return -1;
    }
    ShowWindow(s_appState.hwnd, SW_SHOW);
    UpdateWindow(s_appState.hwnd);

    RECT rc;
    GetClientRect(s_appState.hwnd, &rc);
    unsigned int clientWidth = rc.right - rc.left;
    unsigned int clientHeight = rc.bottom - rc.top;

    {
        std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
        s_appState.buildCommandList.Clear();
        LayoutAndBuildAllTopLevelVisuals(visuals, clientWidth, clientHeight, s_appState.buildCommandList);
        std::swap(s_appState.buildCommandList, s_appState.renderCommandList);
        s_appState.commandListInitialized = true;
    }
    ClearTopLevelVisualsDirty(visuals);

    StartRenderThread(s_appState.hwnd);

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

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        bool needsUpdate = !s_appState.commandListInitialized;
        const auto& visuals = NeoWPF::UI::Visual::GetRegisteredVisuals();
        for (auto visual : visuals) {
            if (!visual->GetParent() && visual->IsTreeDirty()) {
                needsUpdate = true;
                break;
            }
        }

        RECT rc;
        GetClientRect(s_appState.hwnd, &rc);
        unsigned int currentClientWidth = rc.right - rc.left;
        unsigned int currentClientHeight = rc.bottom - rc.top;
        if (currentClientWidth != s_appState.lastClientWidth || currentClientHeight != s_appState.lastClientHeight) {
            needsUpdate = true;
            s_appState.lastClientWidth = currentClientWidth;
            s_appState.lastClientHeight = currentClientHeight;
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
    return s_appState.exitCode;
}

void Application::Shutdown()
{
    s_appState.running = false;
    PostQuitMessage(0);
}

}
