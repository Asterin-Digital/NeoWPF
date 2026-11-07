/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <atomic>
#include <array>
#include <chrono>
#include <cstddef>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Core/Application/ApplicationHost.h>
#include <NeoWPF/Core/Threading/MainThreadExecutor.h>
#include <NeoWPF/Core/Threading/BackgroundExecutor.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Window.h>

namespace NeoWPF::Diagnostics {
struct VisualTreeDumpOptions;
}

namespace NeoWPF::UI {
class Visual;
}

namespace NeoWPF::Core {

struct WindowSession {
    NeoWPF::Rendering::RenderSurface renderSurface;
    SessionLifecycle lifecycle;
    unsigned int width = 0;
    unsigned int height = 0;
    bool needsLayout = true;
    std::shared_ptr<NeoWPF::UI::Window> windowHost;
    NeoWPF::UI::Visual* contentRoot = nullptr;
    NeoWPF::Rendering::CommandList buildCommandList;
    NeoWPF::Rendering::CommandList renderCommandList;
    bool commandListInitialized = false;
};

struct WindowState {
    bool treeDirty = false;
    bool layoutDirty = false;
    std::size_t nodeCount = 0;
    std::size_t dirtyCount = 0;
    std::size_t treeDirtyCount = 0;
    std::size_t layoutDirtyCount = 0;
    std::array<NeoWPF::UI::Visual*, 5> dirtyVisuals{};
    std::size_t sampleCount = 0;
    bool failed = false;
};

struct SurfaceDestroyRequest {
    NeoWPF::Rendering::RenderSurface surface;
    bool completed = false;
};

struct AppState {
    void* hInst = nullptr;
    NeoWPF::Core::ApplicationSession applicationSession;
    std::vector<WindowSession> windowSessions;
    bool hasCreatedWindow = false;
    std::vector<std::shared_ptr<SurfaceDestroyRequest>> pendingSurfaceDestroy;
    std::atomic<bool> running{false};
    std::atomic<bool> consoleVisible{false};
    std::shared_ptr<NeoWPF::UI::Visual> developerWarningOverlayHost;
    std::shared_ptr<NeoWPF::UI::Visual> textInspectionOverlayHost;
    std::atomic<bool> renderThreadInitialized{false};
    std::atomic<bool> renderThreadInitFailed{false};
    bool renderThreadShuttingDown = false;
    std::atomic<bool> layoutDebugLogging{false};
    std::thread renderThread;
    std::unique_ptr<NeoWPF::Core::IApplicationHost> applicationHost;
    std::mutex applicationHostMutex;
    std::unique_ptr<NeoWPF::Core::Threading::MainThreadExecutor> mainThreadExecutor;
    std::unique_ptr<NeoWPF::Core::Threading::BackgroundExecutor> backgroundExecutor;
    int exitCode = 0;

    std::mutex renderInitMutex;
    std::condition_variable renderInitCondition;
    std::mutex renderRequestMutex;
    std::condition_variable renderRequestCondition;
    bool renderRequested = false;
    std::condition_variable surfaceDestroyCondition;

    std::mutex windowSessionsMutex;
    std::mutex windowCreationMutex;
    std::mutex commandListMutex;
    std::vector<NeoWPF::Diagnostics::DeveloperWarningMessage> pendingDeveloperMessages;
    std::vector<std::shared_ptr<NeoWPF::UI::Visual>> developerOverlayVisuals;
    std::vector<std::wstring> pendingTextInspectionLines;
    std::vector<std::shared_ptr<NeoWPF::UI::Visual>> textInspectionOverlayVisuals;
};

/*
    @summary
    Returns the shared application state.

    @internal
*/
AppState& GetAppState();

/*
    @summary
    Checks whether the application loop should continue.

    @internal
*/
bool ShouldContinueLoop(bool hasCreatedWindow, bool anyWindowOpen) noexcept;

/*
    @summary
    Starts the render thread.

    @internal
*/
void StartRenderThread();

/*
    @summary
    Starts rendering and runs the desktop loop.

    @param timeout
    Maximum wait for render initialization.

    @internal
*/
int RunDesktopApp(std::chrono::seconds timeout);

/*
    @summary
    Releases a window surface but keeps its session.

    @param handle
    Stable window handle.

    @returns
    True when release completes for an existing session.

    @internal
*/
bool ReleaseWindowSurface(void* handle);

/*
    @summary
    Attaches a new surface generation to a retained session.

    @param handle
    Stable window handle.

    @returns
    True when the surface attaches and the session activates.

    @internal
*/
bool AttachWindowSurface(void* handle);

/*
    @summary
    Closes and removes a window session.

    @param handle
    Stable window handle.

    @returns
    True when the session is removed.

    @internal
*/
bool CloseWindow(void* handle);

/*
    @summary
    Runs the application message loop.

    @internal
*/
int RunMessageLoop();

/*
    @summary
    Returns the state of a window and its visual tree.

    @param windowSession
    Window session to inspect.

    @internal
*/
WindowState GetWindowState(const WindowSession& windowSession);

/*
    @summary
    Shuts down the application runtime.

    @internal
*/
void ShutdownApp();

/*
    @summary
    Dumps the visual tree.

    @param options
    Controls the dump contents.

    @internal
*/
void DumpTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options);

/*
    @summary
    Selects the input root for a window.

    @param handle
    Stable window handle.

    @internal
*/
void SetInputRoot(void* handle);

}
