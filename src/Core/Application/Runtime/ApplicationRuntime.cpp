#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Threading/ThreadAffinity.h>
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
#include <typeinfo>
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

void LayoutVisualTree(NeoWPF::UI::Visual* rootVisual,
                      const NeoWPF::UI::Layout::LayoutEnvironment& environment)
{
    if (!rootVisual) {
        return;
    }

    if (auto fe = dynamic_cast<NeoWPF::UI::FrameworkElement*>(rootVisual)) {
        fe->Measure(environment.contentWidth, environment.contentHeight);
        fe->Arrange(environment.contentWidth, environment.contentHeight);
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

bool TryGetWindowScaleFactor(
    WindowSession& windowSession,
    NeoWPF::Contracts::IWindowing& windowing,
    double& outScaleFactor)
{
    if (windowing.GetWindowScaleFactor(
            windowSession.renderSurface.nativeWindowHandle, outScaleFactor)) {
        windowSession.scaleFactorFailureReported = false;
        return true;
    }

    windowSession.hasScaleFactor = false;
    if (!windowSession.scaleFactorFailureReported) {
        NeoWPF::Diagnostics::Catastrophic(
            "Window layout and rendering paused because the window scale is unavailable");
        windowSession.scaleFactorFailureReported = true;
    }
    return false;
}

bool TryUpdateLayoutEnvironment(
    WindowSession& windowSession,
    NeoWPF::Contracts::IWindowing& windowing,
    double logicalWidth,
    double logicalHeight)
{
    NeoWPF::Contracts::WindowInsets safeArea{};
    NeoWPF::Contracts::WindowInsets keyboard{};
    NeoWPF::Contracts::InputProfile inputProfile{};

    const bool hasInsets = windowing.GetWindowInsets(
        windowSession.renderSurface.nativeWindowHandle,
        safeArea,
        keyboard);
    const bool hasInputProfile = windowing.GetInputProfile(
        windowSession.renderSurface.nativeWindowHandle,
        inputProfile);

    NeoWPF::UI::Layout::LayoutEnvironment environment;
    environment.contentWidth = logicalWidth;
    environment.contentHeight = logicalHeight;
    environment.scale = windowSession.scaleFactor;

    if (hasInsets) {
        environment.safeArea = NeoWPF::UI::Thickness{
            static_cast<double>(safeArea.left) / windowSession.scaleFactor,
            static_cast<double>(safeArea.top) / windowSession.scaleFactor,
            static_cast<double>(safeArea.right) / windowSession.scaleFactor,
            static_cast<double>(safeArea.bottom) / windowSession.scaleFactor};
        environment.keyboard = NeoWPF::UI::Thickness{
            static_cast<double>(keyboard.left) / windowSession.scaleFactor,
            static_cast<double>(keyboard.top) / windowSession.scaleFactor,
            static_cast<double>(keyboard.right) / windowSession.scaleFactor,
            static_cast<double>(keyboard.bottom) / windowSession.scaleFactor};
    }

    if (hasInputProfile) {
        environment.pointer =
            inputProfile.primaryPointer == NeoWPF::Contracts::PointerPrecision::Coarse
                ? NeoWPF::UI::Layout::PointerPrecision::Coarse
                : NeoWPF::UI::Layout::PointerPrecision::Fine;
        environment.hover = inputProfile.canHover;
    }

    const double keyboardVertical = environment.keyboard.Top + environment.keyboard.Bottom;
    const double keyboardHorizontal = environment.keyboard.Left + environment.keyboard.Right;
    environment.contentWidth = (std::max)(0.0,
        logicalWidth - environment.safeArea.Horizontal() - keyboardHorizontal);
    environment.contentHeight = (std::max)(0.0,
        logicalHeight - environment.safeArea.Vertical() - keyboardVertical);

    const auto previous = windowSession.layoutEnvironment;
    windowSession.layoutEnvironment = environment;
    if (windowSession.windowHost) {
        windowSession.windowHost->SetLayoutEnvironment(environment);
    }
    return previous.contentWidth != environment.contentWidth ||
           previous.contentHeight != environment.contentHeight ||
           previous.safeArea.Left != environment.safeArea.Left ||
           previous.safeArea.Top != environment.safeArea.Top ||
           previous.safeArea.Right != environment.safeArea.Right ||
           previous.safeArea.Bottom != environment.safeArea.Bottom ||
           previous.keyboard.Left != environment.keyboard.Left ||
           previous.keyboard.Top != environment.keyboard.Top ||
           previous.keyboard.Right != environment.keyboard.Right ||
           previous.keyboard.Bottom != environment.keyboard.Bottom ||
           previous.pointer != environment.pointer ||
           previous.hover != environment.hover ||
           previous.scale != environment.scale;
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

WindowState GetWindowState(const WindowSession& windowSession)
{
    WindowState state;
    auto* host = windowSession.windowHost.get();
    if (!host) {
        return state;
    }
    state.treeDirty = host->IsTreeDirty();
    state.layoutDirty = host->IsLayoutDirty();

    try {
        std::vector<NeoWPF::UI::Visual*> pending{host};
        for (std::size_t index = 0; index < pending.size(); ++index) {
            auto* visual = pending[index];
            if (!visual) {
                continue;
            }
            ++state.nodeCount;
            const bool treeDirty = visual->IsTreeDirty();
            const bool layoutDirty = visual->IsLayoutDirty();
            state.treeDirtyCount += treeDirty ? 1u : 0u;
            state.layoutDirtyCount += layoutDirty ? 1u : 0u;
            if (treeDirty || layoutDirty) {
                ++state.dirtyCount;
                if (state.sampleCount < state.dirtyVisuals.size()) {
                    state.dirtyVisuals[state.sampleCount++] = visual;
                }
            }

            const auto children = visual->GetVisualChildren();
            for (auto* child : children) {
                pending.push_back(child);
            }
        }

    } catch (...) {
        state.failed = true;
    }
    return state;
}

void BuildWindowCommands(
    WindowSession& windowSession,
    unsigned int width,
    unsigned int height,
    bool layoutRequired)
{
    NEO_WPF_STARTUP_ACTION("BuildWindowCommands");
    const auto layoutStart = std::chrono::steady_clock::now();
    const double logicalWidth = static_cast<double>(width) / windowSession.scaleFactor;
    const double logicalHeight = static_cast<double>(height) / windowSession.scaleFactor;
    windowSession.buildCommandList.Clear();
    const auto backgroundPaint = NeoWPF::UI::ResolveBrushPaint(
        windowSession.windowHost->GetBackgroundBrush(),
        windowSession.buildCommandList,
        0.0,
        0.0,
        logicalWidth,
        logicalHeight);
    if (backgroundPaint.kind == NeoWPF::Rendering::Paint::Kind::Solid) {
        windowSession.buildCommandList.clearColor = backgroundPaint.color;
    } else {
        windowSession.buildCommandList.clearColor = 0u;
        windowSession.buildCommandList.DrawRect(
            0.0, 0.0, logicalWidth, logicalHeight, backgroundPaint);
    }
    windowSession.buildCommandList.hasClearColor = true;

    auto rootVisual = windowSession.windowHost->GetContent();
    {
        NEO_WPF_ACTION("BuildWindowCommands.Initialize");
        if (rootVisual) {
            rootVisual->Initialize();
        }
    }
    if (layoutRequired) {
        NEO_WPF_ACTION("BuildWindowCommands.MeasureArrange");
        auto* platform = NeoWPF::Platform::CurrentPlatform();
        if (platform) {
            TryUpdateLayoutEnvironment(
                windowSession,
                platform->Windowing(),
                logicalWidth,
                logicalHeight);
        }
        LayoutVisualTree(rootVisual.get(), windowSession.layoutEnvironment);
        for (const auto& overlay : windowSession.windowHost->GetOverlays()) {
            if (!overlay) {
                continue;
            }
            overlay->Initialize();
            if (auto* element = dynamic_cast<NeoWPF::UI::FrameworkElement*>(overlay.get())) {
                element->Measure(
                    windowSession.layoutEnvironment.contentWidth,
                    windowSession.layoutEnvironment.contentHeight);
                element->Arrange(
                    element->GetDesiredWidth(),
                    element->GetDesiredHeight());
            }
        }
    } else {
        NEO_WPF_ACTION("BuildWindowCommands.MeasureArrangeSkipped");
    }
    {
        NEO_WPF_ACTION("BuildWindowCommands.BuildRenderCommands");
        BuildVisualTreeCommands(rootVisual.get(), windowSession.buildCommandList);
        for (const auto& overlay : windowSession.windowHost->GetOverlays()) {
            BuildVisualTreeCommands(overlay.get(), windowSession.buildCommandList);
        }
    }
    const auto layoutElapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - layoutStart).count();
    NeoWPF::Diagnostics::PerformanceMetrics::RecordLayoutPass(layoutElapsed);
    {
        NEO_WPF_ACTION("BuildWindowCommands.PublishCommandList");
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
                        NeoWPF::Diagnostics::Catastrophic("Render backend initialization failed");
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
            NeoWPF::Diagnostics::Catastrophic("Render thread initialization failed: %s", ex.what());
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
            NeoWPF::Diagnostics::Catastrophic("Render thread initialization failed with unknown error");
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
                if (!windowSession.lifecycle.IsActive() ||
                    !windowSession.hasScaleFactor ||
                    !windowSession.renderSurface.IsValid()) {
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
                    NeoWPF::Diagnostics::Catastrophic("Render surface initialization failed: %s", ex.what());
                    continue;
                } catch (...) {
                    NeoWPF::Diagnostics::Catastrophic("Render surface initialization failed with unknown error");
                    continue;
                }
                if (!surfaceInitialized) {
                    NeoWPF::Diagnostics::Catastrophic("Render surface initialization failed");
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

                        NeoWPF::Rendering::Renderer_Execute(
                            surface, toRender, windowSession.scaleFactor);
                        NeoWPF::Rendering::Renderer_Present(surface);
                        const auto renderElapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - renderStart).count();
                        NeoWPF::Diagnostics::PerformanceMetrics::RecordRenderPass(renderElapsed);
                    } catch (const std::exception& ex) {
                        NeoWPF::Diagnostics::Catastrophic("Render frame failed: %s", ex.what());
                    } catch (...) {
                        NeoWPF::Diagnostics::Catastrophic("Render frame failed with unknown error");
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
        } else if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
            platform->Windowing().RequestQuit(0);
        }
    }
    if (s_appState.renderThread.joinable()) {
        s_appState.renderThread.join();
    }
    try {
        NeoWPF::Input::InputManager::GetInstance().Reset();
    } catch (const std::exception& exception) {
        NeoWPF::Diagnostics::Catastrophic(
            "Input state reset failed during application shutdown: %s", exception.what());
    } catch (...) {
        NeoWPF::Diagnostics::Catastrophic(
            "Input state reset failed during application shutdown");
    }
    {
        std::lock_guard<std::mutex> lock(s_appState.applicationHostMutex);
        s_appState.applicationHost.reset();
    }
    {
        std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
        s_appState.windowSessions.clear();
        s_appState.pendingSurfaceDestroy.clear();
        s_appState.hasCreatedWindow = false;
        s_appState.renderThreadShuttingDown = false;
    }
    s_appState.applicationSession.SetContentHost({});
    s_appState.developerWarningOverlayHost.reset();
    s_appState.textInspectionOverlayHost.reset();
    s_appState.developerOverlayVisuals.clear();
    s_appState.textInspectionOverlayVisuals.clear();
    s_appState.pendingDeveloperMessages.clear();
    s_appState.pendingTextInspectionLines.clear();
    s_appState.renderThreadInitialized = false;
    s_appState.renderThreadInitFailed = false;
    s_appState.layoutDebugLogging = false;
    s_appState.exitCode = 0;
    {
        std::lock_guard<std::mutex> lock(s_appState.renderRequestMutex);
        s_appState.renderRequested = false;
    }
    s_appState.surfaceDestroyCondition.notify_all();
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
            double scaleFactor = 0.0;
            if (!TryGetWindowScaleFactor(windowSession, windowing, scaleFactor)) {
                continue;
            }
            windowSession.scaleFactor = scaleFactor;
            windowSession.hasScaleFactor = true;
            BuildWindowCommands(windowSession, windowWidth, windowHeight, true);
            windowSession.width = windowWidth;
            windowSession.height = windowHeight;
            windowSession.needsLayout = false;
            initialWindows.push_back(windowSession.renderSurface.nativeWindowHandle);
        }
        for (const auto& windowSession : s_appState.windowSessions) {
            const auto state = GetWindowState(windowSession);
            NeoWPF::Diagnostics::LogInfo(
                "[state:init] host=%p tree=%d layout=%d nodes=%zu dirty=%zu tree=%zu layout=%zu failed=%d",
                windowSession.windowHost.get(), state.treeDirty ? 1 : 0,
                state.layoutDirty ? 1 : 0, state.nodeCount, state.dirtyCount,
                state.treeDirtyCount, state.layoutDirtyCount, state.failed ? 1 : 0);
            for (std::size_t index = 0; index < state.sampleCount; ++index) {
                auto* visual = state.dirtyVisuals[index];
                NeoWPF::Diagnostics::LogInfo(
                    "[state:init] dirty[%zu]=%s@%p tree=%d layout=%d",
                    index, typeid(*visual).name(), visual,
                    visual->IsTreeDirty() ? 1 : 0, visual->IsLayoutDirty() ? 1 : 0);
            }
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
        static auto lastDirtyProbe = std::chrono::steady_clock::time_point{};
        const auto dirtyProbeTime = std::chrono::steady_clock::now();
        const bool emitDirtyProbe = dirtyProbeTime - lastDirtyProbe >= std::chrono::seconds(1);
        if (emitDirtyProbe) {
            lastDirtyProbe = dirtyProbeTime;
        }
        {
            std::lock_guard<std::mutex> lock(s_appState.windowSessionsMutex);
            for (auto& windowSession : s_appState.windowSessions) {
                const bool lifecycleActive = windowSession.lifecycle.IsActive();
                const bool windowAlive = windowing.IsWindowAlive(windowSession.renderSurface.nativeWindowHandle);
                unsigned int windowWidth = 0;
                unsigned int windowHeight = 0;
                if (windowAlive) {
                    windowing.GetWindowSize(windowSession.renderSurface.nativeWindowHandle, windowWidth, windowHeight);
                }
                double scaleFactor = 0.0;
                const bool hasScaleFactor = windowAlive &&
                    TryGetWindowScaleFactor(windowSession, windowing, scaleFactor);
                const bool scaleChanged = hasScaleFactor &&
                    (!windowSession.hasScaleFactor ||
                     windowSession.scaleFactor != scaleFactor);
                if (scaleChanged) {
                    windowSession.scaleFactor = scaleFactor;
                    windowSession.hasScaleFactor = true;
                    windowSession.needsLayout = true;
                }
                const bool hasPositiveSize = windowWidth > 0 && windowHeight > 0 &&
                    hasScaleFactor;
                const bool widthChanged = windowSession.width != windowWidth;
                const bool heightChanged = windowSession.height != windowHeight;
                const bool hostTreeDirty = windowSession.windowHost && windowSession.windowHost->IsTreeDirty();
                const bool overlayTreeDirty = windowSession.windowHost && OverlayDirty(*windowSession.windowHost);
                if (emitDirtyProbe) {
                    NeoWPF::Diagnostics::LogInfo(
                        "[state:gate] active=%d alive=%d size=%ux%u valid=%d initialized=%d resized=%d/%d hostTree=%d overlayTree=%d",
                        lifecycleActive ? 1 : 0, windowAlive ? 1 : 0, windowWidth, windowHeight,
                        hasPositiveSize ? 1 : 0, windowSession.commandListInitialized ? 1 : 0,
                        widthChanged ? 1 : 0, heightChanged ? 1 : 0,
                        hostTreeDirty ? 1 : 0, overlayTreeDirty ? 1 : 0);
                    const auto state = GetWindowState(windowSession);
                    NeoWPF::Diagnostics::LogInfo(
                        "[state:gate] nodes=%zu dirty=%zu tree=%zu layout=%zu failed=%d",
                        state.nodeCount, state.dirtyCount, state.treeDirtyCount,
                        state.layoutDirtyCount, state.failed ? 1 : 0);
                }
                if (!lifecycleActive || !windowAlive) {
                    continue;
                }
                const bool sessionNeedsUpdate = hasPositiveSize &&
                    (!windowSession.commandListInitialized || windowSession.needsLayout ||
                     widthChanged || heightChanged ||
                     hostTreeDirty || overlayTreeDirty);
                needsUpdate = needsUpdate || sessionNeedsUpdate;
                if (needsUpdate && !emitDirtyProbe) {
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
                double scaleFactor = 0.0;
                if (!TryGetWindowScaleFactor(windowSession, windowing, scaleFactor)) {
                    continue;
                }
                const bool scaleChanged = !windowSession.hasScaleFactor ||
                    windowSession.scaleFactor != scaleFactor;
                if (scaleChanged) {
                    windowSession.scaleFactor = scaleFactor;
                    windowSession.hasScaleFactor = true;
                }
                auto rootVisual = windowSession.windowHost ? windowSession.windowHost->GetContent() : nullptr;
                const bool layoutRequired =
                    !windowSession.commandListInitialized ||
                    windowSession.needsLayout || scaleChanged ||
                    windowSession.width != windowWidth || windowSession.height != windowHeight ||
                    (rootVisual && rootVisual->IsLayoutDirty()) ||
                    (windowSession.windowHost && OverlayLayoutDirty(*windowSession.windowHost));
                BuildWindowCommands(windowSession, windowWidth, windowHeight, layoutRequired);
                windowSession.width = windowWidth;
                windowSession.height = windowHeight;
                windowSession.needsLayout = false;
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
