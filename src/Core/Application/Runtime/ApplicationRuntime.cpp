#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Threading/ThreadAffinity.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
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

bool ShouldContinueLoop(bool hasCreatedWindow, bool anyWindowOpen) noexcept
{
    return !hasCreatedWindow || anyWindowOpen;
}

void SetInputRoot(void* handle)
{
    if (!handle) {
        return;
    }

    std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
    for (auto& windowSession : s_appState.windowSessions) {
        if (windowSession.renderSurface.nativeWindowHandle == handle) {
            auto rootVisual = windowSession.windowHost->GetContent();
            NeoWPF::Input::InputManager::GetInstance().SetRootElement(dynamic_cast<NeoWPF::UI::UIElement*>(rootVisual.get()));
            NeoWPF::Input::InputManager::GetInstance().SetOverlayElements(windowSession.windowHost->GetOverlays());
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
        fe->Measure(width, height);
        fe->Arrange(width, height);
    }
}

void BuildVisualTreeCommands(NeoWPF::UI::Visual* rootVisual, NeoWPF::Rendering::CommandList& buildCommandList)
{
    if (!rootVisual || !NeoWPF::Core::Threading::ThreadAffinity::IsMainThread()) {
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

void BuildWindowVisualTree(WindowSession& windowSession, unsigned int width, unsigned int height, bool layoutRequired)
{
    NEO_WPF_STARTUP_ACTION("BuildWindowVisualTree");
    const auto layoutStart = std::chrono::steady_clock::now();
    windowSession.buildCommandList.Clear();
    const auto clearBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(
        windowSession.windowHost->GetBackgroundBrush());
    windowSession.buildCommandList.clearColor = clearBrush ? clearBrush->GetArgb() : 0u;
    windowSession.buildCommandList.hasClearColor = true;

    auto rootVisual = windowSession.windowHost->GetContent();
    {
        NEO_WPF_ACTION("BuildWindowVisualTree.Initialize");
        if (rootVisual) {
            rootVisual->Initialize();
        }
    }
    if (layoutRequired) {
        NEO_WPF_ACTION("BuildWindowVisualTree.MeasureArrange");
        LayoutVisualTree(rootVisual.get(), width, height);
        for (const auto& overlay : windowSession.windowHost->GetOverlays()) {
            if (!overlay) {
                continue;
            }
            overlay->Initialize();
            if (auto* element = dynamic_cast<NeoWPF::UI::FrameworkElement*>(overlay.get())) {
                element->Measure(width, height);
                element->Arrange(element->GetDesiredWidth(), element->GetDesiredHeight());
            }
        }
    } else {
        NEO_WPF_ACTION("BuildWindowVisualTree.MeasureArrangeSkipped");
    }
    {
        NEO_WPF_ACTION("BuildWindowVisualTree.BuildRenderCommands");
        BuildVisualTreeCommands(rootVisual.get(), windowSession.buildCommandList);
        for (const auto& overlay : windowSession.windowHost->GetOverlays()) {
            BuildVisualTreeCommands(overlay.get(), windowSession.buildCommandList);
        }
    }
    const auto layoutElapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - layoutStart).count();
    NeoWPF::Diagnostics::PerformanceMetrics::RecordLayoutPass(layoutElapsed);
    {
        NEO_WPF_ACTION("BuildWindowVisualTree.PublishCommandList");
        std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
        windowSession.renderCommandList = windowSession.buildCommandList;
    }
    windowSession.commandListInitialized = true;
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
    s_appState.renderThreadShuttingDown = false;
    s_appState.renderThread = std::thread([]() {
    NeoWPF::Core::Threading::ThreadAffinity::RegisterRenderThread();
        auto* platform = NeoWPF::Platform::CurrentPlatform();
        try {
            bool surfacesInitialized = true;
            {
                std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
                for (const auto& windowSession : s_appState.windowSessions) {
                    if (!windowSession.lifecycle.IsActive()) {
                        continue;
                    }
                    if (!platform || !windowSession.renderSurface.IsValid() ||
                        !NeoWPF::Rendering::Renderer_Initialize(windowSession.renderSurface)) {
                        NeoWPF::Diagnostics::LogError("Render backend initialization failed");
                        surfacesInitialized = false;
                        break;
                    }
                }
                if (surfacesInitialized) {
                    s_appState.renderThreadInitialized = true;
                }
            }
            if (!surfacesInitialized) {
                NeoWPF::Rendering::Renderer_Shutdown();
                {
                    std::lock_guard<std::mutex> lock(s_appState.renderInitMutex);
                    s_appState.renderThreadInitFailed = true;
                    s_appState.renderThreadInitialized = false;
                }
                s_appState.running = false;
                s_appState.renderInitCondition.notify_one();
                NeoWPF::Core::Threading::ThreadAffinity::ClearRenderThread();
                return;
            }
            s_appState.renderInitCondition.notify_one();
        }
        catch (const std::exception& ex) {
            NeoWPF::Rendering::Renderer_Shutdown();
            NeoWPF::Diagnostics::LogError("Render thread initialization failed: %s", ex.what());
            {
                std::lock_guard<std::mutex> lock(s_appState.renderInitMutex);
                s_appState.renderThreadInitFailed = true;
                s_appState.renderThreadInitialized = false;
            }
            s_appState.renderInitCondition.notify_one();
            s_appState.running = false;
            NeoWPF::Core::Threading::ThreadAffinity::ClearRenderThread();
            return;
        }
        catch (...) {
            NeoWPF::Rendering::Renderer_Shutdown();
            NeoWPF::Diagnostics::LogError("Render thread initialization failed with unknown error");
            {
                std::lock_guard<std::mutex> lock(s_appState.renderInitMutex);
                s_appState.renderThreadInitFailed = true;
                s_appState.renderThreadInitialized = false;
            }
            s_appState.renderInitCondition.notify_one();
            s_appState.running = false;
            NeoWPF::Core::Threading::ThreadAffinity::ClearRenderThread();
            return;
        }

        while (true) {
            bool keepRendering = false;
            {
                std::unique_lock<std::mutex> lock(s_appState.renderRequestMutex);
                s_appState.renderRequestCondition.wait(lock, [] {
                    return !s_appState.running.load() || s_appState.renderRequested;
                });
                keepRendering = s_appState.running.load();
                if (keepRendering) {
                    s_appState.renderRequested = false;
                }
            }

            std::vector<WindowSession> windowSessionsSnapshot;
            std::vector<std::shared_ptr<SurfaceDestroyRequest>> destroyRequests;
            {
                std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
                destroyRequests.swap(s_appState.pendingSurfaceDestroy);
                if (keepRendering) {
                    windowSessionsSnapshot = s_appState.windowSessions;
                }
            }

            for (const auto& request : destroyRequests) {
                NeoWPF::Rendering::Renderer_DestroySurface(request->surface);
                {
                    std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
                    request->completed = true;
                }
                s_appState.surfaceDestroyCondition.notify_all();
            }
            if (!keepRendering) {
                std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
                if (!s_appState.pendingSurfaceDestroy.empty()) {
                    continue;
                }
                s_appState.renderThreadShuttingDown = true;
                break;
            }

            for (auto& windowSession : windowSessionsSnapshot) {
                if (!windowSession.lifecycle.IsActive() || !windowSession.renderSurface.IsValid()) {
                    continue;
                }
                const auto& surface = windowSession.renderSurface;
                if (!platform->Windowing().IsWindowAlive(surface.nativeWindowHandle)) {
                    continue;
                }
                bool surfaceInitialized = false;
                try {
                    surfaceInitialized = NeoWPF::Rendering::Renderer_Initialize(surface);
                } catch (const std::exception& ex) {
                    NeoWPF::Diagnostics::LogError("Render surface initialization failed: %s", ex.what());
                    continue;
                } catch (...) {
                    NeoWPF::Diagnostics::LogError("Render surface initialization failed with unknown error");
                    continue;
                }
                if (!surfaceInitialized) {
                    NeoWPF::Diagnostics::LogError("Render surface initialization failed");
                    continue;
                }
                unsigned int w = 0;
                unsigned int h = 0;
                platform->Windowing().GetWindowSize(surface.nativeWindowHandle, w, h);

                if (w > 0 && h > 0) {
                    try {
                        NeoWPF::Rendering::Renderer_Resize(surface, w, h);
                        const auto renderStart = std::chrono::steady_clock::now();
                        NeoWPF::Rendering::Renderer_BeginFrame(surface);

                        NeoWPF::Rendering::CommandList toRender;
                        {
                            std::lock_guard<std::mutex> lock(s_appState.commandListMutex);
                            toRender = windowSession.renderCommandList;
                        }

                        NeoWPF::Rendering::Renderer_Execute(surface, toRender);
                        NeoWPF::Rendering::Renderer_Present(surface);
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

        NeoWPF::Rendering::Renderer_Shutdown();
        {
            std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
            s_appState.renderThreadInitialized = false;
            s_appState.renderThreadShuttingDown = false;
        }
        s_appState.surfaceDestroyCondition.notify_all();
        NeoWPF::Core::Threading::ThreadAffinity::ClearRenderThread();
    });
}

void ShutdownApp()
{
    s_appState.running = false;
    s_appState.renderRequestCondition.notify_all();
    s_appState.applicationSession.Shutdown();
    {
        std::lock_guard<std::mutex> lock(s_appState.applicationHostMutex);
        if (s_appState.applicationHost) {
            s_appState.applicationHost->Shutdown();
            s_appState.applicationHost->RequestQuit(0);
        } else {
            NeoWPF::Platform::CurrentPlatform()->Windowing().RequestQuit(0);
        }
    }
}

int RunMessageLoop()
{
    NeoWPF::Core::Threading::ThreadAffinity::RegisterMainThread();
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    auto& windowing = platform->Windowing();
    std::vector<void*> initialWindows;
    {
        std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
        for (auto& windowSession : s_appState.windowSessions) {
            if (!windowSession.lifecycle.IsActive() ||
                !windowing.IsWindowAlive(windowSession.renderSurface.nativeWindowHandle)) {
                continue;
            }
            unsigned int windowWidth = 0;
            unsigned int windowHeight = 0;
            windowing.GetWindowSize(windowSession.renderSurface.nativeWindowHandle, windowWidth, windowHeight);
            if (windowWidth == 0 || windowHeight == 0) {
                continue;
            }
            BuildWindowVisualTree(windowSession, windowWidth, windowHeight, true);
            windowSession.width = windowWidth;
            windowSession.height = windowHeight;
            initialWindows.push_back(windowSession.renderSurface.nativeWindowHandle);
        }
    }

    for (void* handle : initialWindows) {
        windowing.InvalidateWindow(handle);
    }
    if (!initialWindows.empty()) {
        SetInputRoot(initialWindows.front());
    }

    NeoWPF::Diagnostics::PerformanceMetrics::RecordStartup();

    auto applicationHost = std::make_unique<NeoWPF::Core::DesktopApplicationHost>(
        windowing, s_appState.applicationSession);
    NeoWPF::Core::IApplicationHost* applicationHostPointer = applicationHost.get();
    {
        std::lock_guard<std::mutex> lock(s_appState.applicationHostMutex);
        s_appState.applicationHost = std::move(applicationHost);
    }
    s_appState.exitCode = applicationHostPointer->Run([&] {

        if (s_appState.mainThreadExecutor) {
            s_appState.mainThreadExecutor->Drain(std::chrono::milliseconds(2));
        }

        const auto tickStart = std::chrono::steady_clock::now();
        NeoWPF::UI::Tick();
        const auto tickElapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - tickStart).count();
        NeoWPF::Diagnostics::PerformanceMetrics::RecordMessageLoopTick(tickElapsed);
        NeoWPF::Diagnostics::PerformanceMetrics::RecordFrameSample(tickElapsed);
        NeoWPF::Diagnostics::PerformanceMetrics::EmitPeriodicSummary();

        bool anyWindowOpen = false;
        bool hasCreatedWindow = false;
        {
            std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
            hasCreatedWindow = s_appState.hasCreatedWindow;
            for (const auto& window : s_appState.windowSessions) {
                if (windowing.IsWindowAlive(window.renderSurface.nativeWindowHandle)) {
                    anyWindowOpen = true;
                    break;
                }
            }
        }
        if (!ShouldContinueLoop(hasCreatedWindow, anyWindowOpen)) {
            return false;
        }

        bool needsUpdate = false;
        {
            std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
            for (auto& windowSession : s_appState.windowSessions) {
                if (!windowSession.lifecycle.IsActive() ||
                    !windowing.IsWindowAlive(windowSession.renderSurface.nativeWindowHandle)) {
                    continue;
                }
                unsigned int windowWidth = 0;
                unsigned int windowHeight = 0;
                windowing.GetWindowSize(windowSession.renderSurface.nativeWindowHandle, windowWidth, windowHeight);
                if (windowWidth > 0 && windowHeight > 0 &&
                    (!windowSession.commandListInitialized || windowSession.width != windowWidth ||
                     windowSession.height != windowHeight ||
                     (windowSession.windowHost &&
                      (windowSession.windowHost->IsTreeDirty() || OverlayDirty(*windowSession.windowHost))))) {
                    needsUpdate = true;
                    break;
                }
            }
        }

        if (needsUpdate) {
            NeoWPF::Diagnostics::ProcessPendingDeveloperWarnings();
            std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
            for (auto& windowSession : s_appState.windowSessions) {
                if (!windowSession.lifecycle.IsActive() ||
                    !windowing.IsWindowAlive(windowSession.renderSurface.nativeWindowHandle)) {
                    continue;
                }
                unsigned int windowWidth = 0;
                unsigned int windowHeight = 0;
                windowing.GetWindowSize(windowSession.renderSurface.nativeWindowHandle, windowWidth, windowHeight);
                if (windowWidth == 0 || windowHeight == 0) {
                    continue;
                }
                auto rootVisual = windowSession.windowHost ? windowSession.windowHost->GetContent() : nullptr;
                const bool layoutRequired =
                    !windowSession.commandListInitialized ||
                    windowSession.width != windowWidth || windowSession.height != windowHeight ||
                    (rootVisual && rootVisual->IsLayoutDirty()) ||
                    (windowSession.windowHost && OverlayLayoutDirty(*windowSession.windowHost));
                BuildWindowVisualTree(windowSession, windowWidth, windowHeight, layoutRequired);
                windowSession.width = windowWidth;
                windowSession.height = windowHeight;
            }
            for (auto& windowSession : s_appState.windowSessions) {
                if (windowSession.windowHost) {
                    ClearVisualTreeDirtyForRoot(windowSession.windowHost.get());
                    for (const auto& overlay : windowSession.windowHost->GetOverlays()) {
                        ClearVisualTreeDirtyForRoot(overlay.get());
                    }
                }
            }
        }
        return true;
    });
    {
        std::lock_guard<std::mutex> lock(s_appState.applicationHostMutex);
        s_appState.applicationHost.reset();
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "Application message loop exiting");
    s_appState.running = false;
    s_appState.renderRequestCondition.notify_all();
    if (s_appState.renderThread.joinable()) {
        s_appState.renderThread.join();
    }

    {
        std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
        s_appState.windowSessions.clear();
    }

    return s_appState.exitCode;
}

}
