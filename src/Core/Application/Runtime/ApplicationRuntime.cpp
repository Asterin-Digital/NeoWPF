#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
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
            NeoWPF::Input::InputManager::GetInstance().SetOverlayElements(windowState.windowHost->GetOverlays());
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
    rootVisual->ClearLayoutDirty();
    if (auto uiElement = dynamic_cast<NeoWPF::UI::UIElement*>(rootVisual)) {
        uiElement->ClearDirty();
    }

    for (NeoWPF::UI::Visual* child : rootVisual->GetVisualChildren()) {
        ClearVisualTreeDirtyForRoot(child);
    }
}

void LayoutVisualTree(NeoWPF::UI::Visual* rootVisual, unsigned int width, unsigned int height)
{
    if (!rootVisual) {
        return;
    }

    if (auto fe = dynamic_cast<NeoWPF::UI::FrameworkElement*>(rootVisual)) {
        fe->Measure(static_cast<int>(width), static_cast<int>(height));
        fe->Arrange(static_cast<int>(width), static_cast<int>(height));
    }
}

void BuildVisualTreeCommands(NeoWPF::UI::Visual* rootVisual, NeoWPF::Rendering::CommandList& buildCommandList)
{
    if (!rootVisual) {
        return;
    }

    //the renderer may not be ready yet, check before building commands to avoid crashes
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (platform && platform->Rendering().IsInitialized()) {
        rootVisual->BuildRenderCommands(buildCommandList);
    }
}

bool OverlayDirty(const NeoWPF::UI::Window& window)
{
    for (const auto& overlay : window.GetOverlays()) {
        if (overlay && overlay->IsTreeDirty()) {
            return true;
        }
    }
    return false;
}

bool OverlayLayoutDirty(const NeoWPF::UI::Window& window)
{
    for (const auto& overlay : window.GetOverlays()) {
        if (overlay && overlay->IsLayoutDirty()) {
            return true;
        }
    }
    return false;
}

}

void BuildWindowVisualTree(WindowState& windowState, unsigned int width, unsigned int height, bool layoutRequired)
{
    NEO_WPF_STARTUP_ACTION("BuildWindowVisualTree");
    const auto layoutStart = std::chrono::steady_clock::now();
    windowState.buildCommandList.Clear();
    windowState.buildCommandList.clearColor = windowState.windowHost->GetBackgroundColor().ToArgb();
    windowState.buildCommandList.hasClearColor = true;

    auto rootVisual = windowState.windowHost->GetContent();
    {
        NEO_WPF_ACTION("BuildWindowVisualTree.Initialize");
        if (rootVisual) {
            rootVisual->Initialize();
        }
    }
    if (layoutRequired) {
        NEO_WPF_ACTION("BuildWindowVisualTree.MeasureArrange");
        LayoutVisualTree(rootVisual.get(), width, height);
        for (const auto& overlay : windowState.windowHost->GetOverlays()) {
            if (!overlay) {
                continue;
            }
            overlay->Initialize();
            if (auto* element = dynamic_cast<NeoWPF::UI::FrameworkElement*>(overlay.get())) {
                element->Measure(static_cast<int>(width), static_cast<int>(height));
                element->Arrange(element->GetDesiredWidth(), element->GetDesiredHeight());
            }
        }
    } else {
        NEO_WPF_ACTION("BuildWindowVisualTree.MeasureArrangeSkipped");
    }
    {
        NEO_WPF_ACTION("BuildWindowVisualTree.BuildRenderCommands");
        BuildVisualTreeCommands(rootVisual.get(), windowState.buildCommandList);
        for (const auto& overlay : windowState.windowHost->GetOverlays()) {
            BuildVisualTreeCommands(overlay.get(), windowState.buildCommandList);
        }
    }
    const auto layoutElapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - layoutStart).count();
    NeoWPF::Diagnostics::PerformanceMetrics::RecordLayoutPass(layoutElapsed);
    {
        NEO_WPF_ACTION("BuildWindowVisualTree.PublishCommandList");
        std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
        windowState.renderCommandList = windowState.buildCommandList;
    }
    windowState.commandListInitialized = true;
    {
        std::lock_guard<std::mutex> lock(s_appState.renderRequestMutex);
        s_appState.renderRequested = true;
    }
    s_appState.renderRequestCondition.notify_one();
}

void StartRenderThread()
{
    NEO_WPF_STARTUP_ACTION("RenderThread.Start");
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "StartRenderThread");
    s_appState.running = true;
    s_appState.renderThreadInitialized = false;
    s_appState.renderThreadInitFailed = false;
    s_appState.renderThread = std::thread([]() {
        auto* platform = NeoWPF::Platform::CurrentPlatform();
        try {
            {
                std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
                for (const auto& windowState : s_appState.windows) {
                    if (!platform || !platform->Rendering().Initialize(windowState.nativeWindowHandle)) {
                        NeoWPF::Diagnostics::LogError("Render backend initialization failed");
                        std::lock_guard<std::mutex> initLock(s_appState.renderInitMutex);
                        s_appState.renderThreadInitFailed = true;
                        s_appState.renderThreadInitialized = false;
                        s_appState.running = false;
                        s_appState.renderInitCondition.notify_one();
                        return;
                    }
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
            {
                std::unique_lock<std::mutex> lock(s_appState.renderRequestMutex);
                s_appState.renderRequestCondition.wait(lock, [] {
                    return !s_appState.running.load() || s_appState.renderRequested;
                });
                if (!s_appState.running.load()) {
                    break;
                }
                s_appState.renderRequested = false;
            }

            std::vector<WindowState> windowsSnapshot;
            {
                std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
                windowsSnapshot = s_appState.windows;
            }

            for (auto& windowState : windowsSnapshot) {
                unsigned int w = 0;
                unsigned int h = 0;
                if (!platform->Windowing().IsWindowAlive(windowState.nativeWindowHandle)) {
                    continue;
                }
                platform->Windowing().GetWindowSize(windowState.nativeWindowHandle, w, h);

                if (w > 0 && h > 0) {
                    try {
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

        }

        {
            std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
            for (const auto& windowState : s_appState.windows) {
                platform->Rendering().Shutdown();
            }
        }
    });
}

void ShutdownApplication()
{
    s_appState.running = false;
    s_appState.renderRequestCondition.notify_all();
    NeoWPF::Platform::CurrentPlatform()->Windowing().RequestQuit(0);
}

int RunApplicationMessageLoop()
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    auto& windowing = platform->Windowing();
    void* primaryWindow = nullptr;
    {
        std::lock_guard<std::mutex> wlock(s_appState.windowsMutex);
        if (!s_appState.windows.empty()) {
            primaryWindow = s_appState.windows[0].nativeWindowHandle;
        }
    }

    if (!primaryWindow) {
        NeoWPF::Platform::CurrentPlatform()->Windowing().ShowFatalError(
            L"NeoWPF Error", L"No primary window available.");
        s_appState.running = false;
        return -1;
    }

    unsigned int clientWidth = 0;
    unsigned int clientHeight = 0;
    windowing.GetWindowSize(primaryWindow, clientWidth, clientHeight);

    const auto& visuals = NeoWPF::UI::Visual::GetRegisteredVisuals();
    {
        std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
        for (auto& windowState : s_appState.windows) {
            unsigned int windowWidth = 0;
            unsigned int windowHeight = 0;
            if (!windowing.IsWindowAlive(windowState.nativeWindowHandle)) {
                continue;
            }
            windowing.GetWindowSize(windowState.nativeWindowHandle, windowWidth, windowHeight);
            if (windowWidth == 0 || windowHeight == 0) {
                windowWidth = clientWidth;
                windowHeight = clientHeight;
            }
            BuildWindowVisualTree(windowState, windowWidth, windowHeight, true);
            windowState.width = windowWidth;
            windowState.height = windowHeight;
        }
    }

    if (!s_appState.windows.empty()) {
        auto rootVisual = s_appState.windows[0].windowHost->GetContent();
        NeoWPF::Input::InputManager::GetInstance().SetRootElement(dynamic_cast<NeoWPF::UI::UIElement*>(rootVisual.get()));
    }

    windowing.InvalidateWindow(primaryWindow);

    NeoWPF::Diagnostics::PerformanceMetrics::RecordStartup();

    unsigned int lastLayoutWidth = 0, lastLayoutHeight = 0;
    constexpr auto frameInterval = std::chrono::milliseconds(16);
    auto nextFrame = std::chrono::steady_clock::now();
    bool quitRequested = false;

    while (!quitRequested) {
        const auto now = std::chrono::steady_clock::now();
        if (now < nextFrame) {
            const auto waitMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(nextFrame - now);
            if (!windowing.PumpMessages(waitMilliseconds)) {
                quitRequested = true;
            }
        } else if (!windowing.PumpMessages(std::chrono::milliseconds(0))) {
            quitRequested = true;
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
                if (windowing.IsWindowAlive(window.nativeWindowHandle)) {
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
            if (windowState.windowHost &&
                (windowState.windowHost->IsTreeDirty() || OverlayDirty(*windowState.windowHost))) {
                anyTopLevelDirty = true;
                break;
            }
        }

        unsigned int currentClientWidth = 0;
        unsigned int currentClientHeight = 0;
        windowing.GetWindowSize(primaryWindow, currentClientWidth, currentClientHeight);
        const bool sizeChanged = currentClientWidth != lastLayoutWidth || currentClientHeight != lastLayoutHeight;
        if (sizeChanged) {
            lastLayoutWidth = currentClientWidth;
            lastLayoutHeight = currentClientHeight;
        }

        {
            std::lock_guard<std::mutex> lock(s_appState.windowsMutex);
            for (auto& windowState : s_appState.windows) {
                unsigned int windowWidth = 0;
                unsigned int windowHeight = 0;
                if (!windowing.IsWindowAlive(windowState.nativeWindowHandle)) {
                    continue;
                }
                windowing.GetWindowSize(windowState.nativeWindowHandle, windowWidth, windowHeight);
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
                unsigned int windowWidth = 0;
                unsigned int windowHeight = 0;
                if (!windowing.IsWindowAlive(windowState.nativeWindowHandle)) {
                    continue;
                }
                windowing.GetWindowSize(windowState.nativeWindowHandle, windowWidth, windowHeight);
                if (windowWidth == 0 || windowHeight == 0) {
                    windowWidth = currentClientWidth;
                    windowHeight = currentClientHeight;
                }
                auto rootVisual = windowState.windowHost ? windowState.windowHost->GetContent() : nullptr;
                const bool layoutRequired = sizeChanged ||
                    !windowState.commandListInitialized ||
                    (rootVisual && rootVisual->IsLayoutDirty()) ||
                    OverlayLayoutDirty(*windowState.windowHost);
                BuildWindowVisualTree(windowState, windowWidth, windowHeight, layoutRequired);
                windowState.width = windowWidth;
                windowState.height = windowHeight;
            }
            for (auto& windowState : s_appState.windows) {
                if (windowState.windowHost) {
                    ClearVisualTreeDirtyForRoot(windowState.windowHost.get());
                    for (const auto& overlay : windowState.windowHost->GetOverlays()) {
                        ClearVisualTreeDirtyForRoot(overlay.get());
                    }
                }
            }
        }
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "Application message loop exiting");
    s_appState.exitCode = windowing.ExitCode();
    s_appState.running = false;
    s_appState.renderRequestCondition.notify_all();
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
