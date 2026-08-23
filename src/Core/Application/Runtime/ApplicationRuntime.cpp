#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <chrono>
#include <exception>
#include <mutex>
#include <utility>
#include <vector>

namespace NeoWPF::Core {
namespace {

AppState s_appState;

}

AppState& GetAppState() { return s_appState; }

void SetInputRootForWindow(void* nativeWindowHandle)
{
    if (!nativeWindowHandle) {
        return;
    }

    std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
    for (auto& windowState : s_appState.windows) {
        if (windowState.nativeWindowHandle == nativeWindowHandle) {
            auto rootVisual = windowState.windowHost->GetContent();
            NeoWPF::Input::InputManager::GetInstance().SetRootElement(dynamic_cast<NeoWPF::UI::UIElement*>(rootVisual.get()));
            return;
        }
    }
}

namespace {

void ClearVisualTreeDirtyForRoot(NeoWPF::UI::Visual* rootVisual)
{
    if (!rootVisual) {
        return;
    }

    rootVisual->ClearTreeDirty();
    if (auto uiElement = dynamic_cast<NeoWPF::UI::UIElement*>(rootVisual)) {
        uiElement->ClearDirty();
    }

    for (NeoWPF::UI::Visual* child : rootVisual->GetVisualChildren()) {
        ClearVisualTreeDirtyForRoot(child);
    }
}

void LayoutAndBuildVisualTree(NeoWPF::UI::Visual* rootVisual, unsigned int width, unsigned int height, NeoWPF::Rendering::CommandList& buildCommandList)
{
    if (!rootVisual) {
        return;
    }

    if (auto fe = dynamic_cast<NeoWPF::UI::FrameworkElement*>(rootVisual)) {
        fe->Measure(static_cast<int>(width), static_cast<int>(height));
        fe->Arrange(static_cast<int>(width), static_cast<int>(height));
    }
    
    //the renderer may not be ready yet, check before building commands to avoid crashes
    if (NeoWPF::Rendering::Renderer_IsInitialized()) {
        rootVisual->BuildRenderCommands(buildCommandList);
    }
}

}

void BuildWindowVisualTree(WindowState& windowState, unsigned int width, unsigned int height)
{
    NEO_WPF_STARTUP_ACTION("BuildWindowVisualTree");
    const auto layoutStart = std::chrono::steady_clock::now();
    windowState.buildCommandList.Clear();

    auto rootVisual = windowState.windowHost->GetContent();
    if (rootVisual) {
        rootVisual->Initialize();
    }
    LayoutAndBuildVisualTree(rootVisual.get(), width, height, windowState.buildCommandList);
    const auto layoutElapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - layoutStart).count();
    NeoWPF::Diagnostics::PerformanceMetrics::RecordLayoutPass(layoutElapsed);
    {
        std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
        windowState.renderCommandList = windowState.buildCommandList;
    }
    windowState.commandListInitialized = true;
}

void StartRenderThread()
{
    NEO_WPF_STARTUP_ACTION("RenderThread.Start");
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "StartRenderThread");
    s_appState.running = true;
    s_appState.renderThreadInitialized = false;
    s_appState.renderThreadInitFailed = false;
    s_appState.renderThread = std::thread([]() {
        try {
            {
                std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
                for (const auto& windowState : s_appState.windows) {
                    NeoWPF::Rendering::Renderer_Initialize(windowState.nativeWindowHandle);
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

            for (auto& windowState : windowsSnapshot) {
                HWND hwnd = static_cast<HWND>(windowState.nativeWindowHandle);
                if (!IsWindow(hwnd)) {
                    continue;
                }

                RECT rc;
                GetClientRect(hwnd, &rc);
                unsigned int w = rc.right - rc.left;
                unsigned int h = rc.bottom - rc.top;

                if (w > 0 && h > 0) {
                    try {
                        NeoWPF::Rendering::Renderer_Initialize(windowState.nativeWindowHandle);
                        NeoWPF::Rendering::Renderer_Resize(w, h);
                        const auto renderStart = std::chrono::steady_clock::now();
                        NeoWPF::Rendering::Renderer_BeginFrame();

                        NeoWPF::Rendering::CommandList toRender;
                        {
                            std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
                            toRender = windowState.renderCommandList;
                        }

                        NeoWPF::Rendering::Renderer_Execute(toRender);
                        NeoWPF::Rendering::Renderer_Present();
                        const auto renderElapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - renderStart).count();
                        NeoWPF::Diagnostics::PerformanceMetrics::RecordRenderPass(renderElapsed);
                    } catch (const std::exception& ex) {
                        NeoWPF::Diagnostics::LogError("Render frame failed: %s", ex.what());
                    } catch (...) {
                        NeoWPF::Diagnostics::LogError("Render frame failed with unknown error");
                    }
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
            primaryWindow = static_cast<HWND>(s_appState.windows[0].nativeWindowHandle);
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
        std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
        for (auto& windowState : s_appState.windows) {
            HWND hwnd = static_cast<HWND>(windowState.nativeWindowHandle);
            if (!IsWindow(hwnd)) {
                continue;
            }

            RECT windowRect;
            GetClientRect(hwnd, &windowRect);
            unsigned int windowWidth = windowRect.right - windowRect.left;
            unsigned int windowHeight = windowRect.bottom - windowRect.top;
            if (windowWidth == 0 || windowHeight == 0) {
                windowWidth = clientWidth;
                windowHeight = clientHeight;
            }
            BuildWindowVisualTree(windowState, windowWidth, windowHeight);
            windowState.width = windowWidth;
            windowState.height = windowHeight;
        }
    }

    if (!s_appState.windows.empty()) {
        auto rootVisual = s_appState.windows[0].windowHost->GetContent();
        NeoWPF::Input::InputManager::GetInstance().SetRootElement(dynamic_cast<NeoWPF::UI::UIElement*>(rootVisual.get()));
    }

    ::UpdateWindow(primaryWindow);
    ::RedrawWindow(primaryWindow, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);

    NeoWPF::Diagnostics::PerformanceMetrics::RecordStartup();

    MSG msg{};
    unsigned int lastLayoutWidth = 0, lastLayoutHeight = 0;
    constexpr auto frameInterval = std::chrono::milliseconds(16);
    auto nextFrame = std::chrono::steady_clock::now();
    bool quitRequested = false;

    while (!quitRequested) {
        const auto now = std::chrono::steady_clock::now();
        if (now < nextFrame) {
            const auto waitMilliseconds = static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(nextFrame - now).count());
            const DWORD waitResult = MsgWaitForMultipleObjects(0, nullptr, FALSE, waitMilliseconds, QS_ALLINPUT);
            if (waitResult == WAIT_FAILED) {
                break;
            }
        }

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                quitRequested = true;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (quitRequested) {
            break;
        }

        nextFrame = std::chrono::steady_clock::now() + frameInterval;

        const auto tickStart = std::chrono::steady_clock::now();
        NeoWPF::UI::Tick();
        const auto tickElapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - tickStart).count();
        NeoWPF::Diagnostics::PerformanceMetrics::RecordMessageLoopTick(tickElapsed);
        NeoWPF::Diagnostics::PerformanceMetrics::RecordFrameSample(tickElapsed);
        NeoWPF::Diagnostics::PerformanceMetrics::EmitPeriodicSummary();

        {
            std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
            bool anyWindowOpen = false;
            for (const auto& window : s_appState.windows) {
                HWND hwnd = static_cast<HWND>(window.nativeWindowHandle);
                if (IsWindow(hwnd)) {
                    anyWindowOpen = true;
                    break;
                }
            }
            if (!anyWindowOpen) {
                break;
            }
        }

        bool needsUpdate = false;
        bool anyTopLevelDirty = false;
        for (const auto& windowState : s_appState.windows) {
            if (windowState.windowHost && windowState.windowHost->GetContent() && windowState.windowHost->GetContent()->IsTreeDirty()) {
                anyTopLevelDirty = true;
                break;
            }
        }

        RECT rc;
        GetClientRect(primaryWindow, &rc);
        unsigned int currentClientWidth = rc.right - rc.left;
        unsigned int currentClientHeight = rc.bottom - rc.top;
        const bool sizeChanged = currentClientWidth != lastLayoutWidth || currentClientHeight != lastLayoutHeight;
        if (sizeChanged) {
            lastLayoutWidth = currentClientWidth;
            lastLayoutHeight = currentClientHeight;
        }

        {
            std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
            for (auto& windowState : s_appState.windows) {
                HWND hwnd = static_cast<HWND>(windowState.nativeWindowHandle);
                if (!IsWindow(hwnd)) {
                    continue;
                }

                RECT windowRect;
                GetClientRect(hwnd, &windowRect);
                unsigned int windowWidth = windowRect.right - windowRect.left;
                unsigned int windowHeight = windowRect.bottom - windowRect.top;
                if (windowWidth == 0 || windowHeight == 0) {
                    windowWidth = currentClientWidth;
                    windowHeight = currentClientHeight;
                }

                if (!windowState.commandListInitialized || windowState.width != windowWidth || windowState.height != windowHeight) {
                    needsUpdate = true;
                    break;
                }
            }
        }

        needsUpdate = needsUpdate || anyTopLevelDirty || sizeChanged;

        if (needsUpdate) {
            NeoWPF::Diagnostics::ProcessPendingDeveloperWarnings();
            std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
            for (auto& windowState : s_appState.windows) {
                HWND hwnd = static_cast<HWND>(windowState.nativeWindowHandle);
                if (!IsWindow(hwnd)) {
                    continue;
                }

                RECT windowRect;
                GetClientRect(hwnd, &windowRect);
                unsigned int windowWidth = windowRect.right - windowRect.left;
                unsigned int windowHeight = windowRect.bottom - windowRect.top;
                if (windowWidth == 0 || windowHeight == 0) {
                    windowWidth = currentClientWidth;
                    windowHeight = currentClientHeight;
                }
                BuildWindowVisualTree(windowState, windowWidth, windowHeight);
                windowState.width = windowWidth;
                windowState.height = windowHeight;
            }
            for (auto& windowState : s_appState.windows) {
                if (windowState.windowHost && windowState.windowHost->GetContent()) {
                    ClearVisualTreeDirtyForRoot(windowState.windowHost->GetContent().get());
                }
            }
        }
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "Application message loop exiting");
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
