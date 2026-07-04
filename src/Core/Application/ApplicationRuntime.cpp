#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Root/RootVisuals.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/Window.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/Input/InputManager.h>
#include <chrono>
#include <exception>
#include <mutex>
#include <utility>
#include <vector>

namespace NeoWPF::Core {
namespace {

AppState s_appState;

} // namespace

AppState& GetAppState() { return s_appState; }

void StartRenderThread()
{
    s_appState.running = true;
    s_appState.renderThreadInitialized = false;
    s_appState.renderThreadInitFailed = false;
    s_appState.renderThread = std::thread([]() {
        try {
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

        while (s_appState.running.load()) {
            std::vector<WindowState> windowsSnapshot;
            {
                std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
                windowsSnapshot = s_appState.windows;
            }

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

        {
            std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
            for (const auto& windowState : s_appState.windows) {
                NeoWPF::Rendering::Renderer_Shutdown();
            }
        }
    });
}

void ShutdownApplication()
{
    s_appState.running = false;
    PostQuitMessage(0);
}

int RunApplicationMessageLoop()
{
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

    ::ShowWindow(primaryWindow, SW_SHOW);
    ::UpdateWindow(primaryWindow);
    ::RedrawWindow(primaryWindow, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);

    MSG msg;
    unsigned int lastLayoutWidth = 0, lastLayoutHeight = 0;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

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

    {
        std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
        s_appState.windows.clear();
    }

    return s_appState.exitCode;
}

}
