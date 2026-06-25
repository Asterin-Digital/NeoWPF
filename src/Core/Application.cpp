#include <NeoWPF/Core/Application.h>
#include <NeoWPF/Core/Root/RootVisuals.h>
#include <NeoWPF/Platform/Window.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Visual.h>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <NeoWPF/UI/Controls/Window.h>
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
    std::thread renderThread;
    int exitCode = 0;

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
    s_appState.renderThread = std::thread([hwnd]() {
        try {
            NeoWPF::Rendering::Renderer_Initialize(hwnd);
        }
        catch (...) {
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
    StartRenderThread(s_appState.hwnd);

    RECT rc;
    GetClientRect(s_appState.hwnd, &rc);
    unsigned int w = rc.right - rc.left;
    unsigned int h = rc.bottom - rc.top;

    std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
    s_appState.buildCommandList.Clear();
    if (rootVisual) {
        LayoutAndBuildVisualTree(rootVisual, w, h, s_appState.buildCommandList);
    } else {
        LayoutAndBuildAllTopLevelVisuals(visuals, w, h, s_appState.buildCommandList);
    }
    std::swap(s_appState.buildCommandList, s_appState.renderCommandList);
    s_appState.commandListInitialized = true;
    if (rootVisual) {
        ClearTopLevelVisualsDirty({rootVisual});
    } else {
        ClearTopLevelVisualsDirty(visuals);
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

        if (needsUpdate) {
            RECT rc;
            GetClientRect(s_appState.hwnd, &rc);
            unsigned int w = rc.right - rc.left;
            unsigned int h = rc.bottom - rc.top;

            std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
            s_appState.buildCommandList.Clear();
            const auto rootInfo = FindRootVisual(visuals);
            if (rootInfo.rootVisual) {
                LayoutAndBuildVisualTree(rootInfo.rootVisual, w, h, s_appState.buildCommandList);
            } else {
                LayoutAndBuildAllTopLevelVisuals(visuals, w, h, s_appState.buildCommandList);
            }
            std::swap(s_appState.buildCommandList, s_appState.renderCommandList);
            s_appState.commandListInitialized = true;
            if (rootInfo.rootVisual) {
                ClearTopLevelVisualsDirty({rootInfo.rootVisual});
            } else {
                ClearTopLevelVisualsDirty(visuals);
            }
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
