#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <algorithm>
#include <mutex>

namespace NeoWPF::Core {
namespace {

AppState& App() { return GetAppState(); }

struct DestroyWait {
    std::shared_ptr<SurfaceDestroyRequest> request;
    bool waitForShutdown = false;
};

DestroyWait QueueDestroyLocked(WindowSession& session)
{
    DestroyWait wait;
    session.lifecycle.Deactivate();

    const auto surface = session.renderSurface;
    if (surface.IsValid()) {
        surface.Invalidate();
        if (App().renderThreadInitialized.load()) {
            wait.waitForShutdown = App().renderThreadShuttingDown;
            if (!wait.waitForShutdown) {
                wait.request = std::make_shared<SurfaceDestroyRequest>();
                wait.request->surface = surface;
                App().pendingSurfaceDestroy.push_back(wait.request);
            }
        }
        return wait;
    }

    const auto pending = std::find_if(
        App().pendingSurfaceDestroy.begin(),
        App().pendingSurfaceDestroy.end(),
        [&surface](const auto& request) {
            return request->surface.nativeWindowHandle == surface.nativeWindowHandle &&
                   request->surface.GetGenerationId() == surface.GetGenerationId();
        });
    if (pending != App().pendingSurfaceDestroy.end()) {
        wait.request = *pending;
    } else {
        wait.waitForShutdown = App().renderThreadInitialized.load() &&
            App().renderThreadShuttingDown;
    }
    return wait;
}

void WaitDestroy(const DestroyWait& wait)
{
    if (wait.request) {
        {
            std::lock_guard<std::mutex> lock(App().renderRequestMutex);
            App().renderRequested = true;
        }
        App().renderRequestCondition.notify_one();
        std::unique_lock<std::mutex> lock(App().windowSessionsMutex);
        App().surfaceDestroyCondition.wait(lock, [&wait] {
            return wait.request->completed;
        });
    } else if (wait.waitForShutdown) {
        std::unique_lock<std::mutex> lock(App().windowSessionsMutex);
        App().surfaceDestroyCondition.wait(lock, [] {
            return !App().renderThreadInitialized.load();
        });
    }
}

}

bool ReleaseWindowSurface(void* handle)
{
    if (!handle) {
        return false;
    }

    DestroyWait wait;
    {
        std::lock_guard<std::mutex> lock(App().windowSessionsMutex);
        const auto session = std::find_if(
            App().windowSessions.begin(), App().windowSessions.end(),
            [handle](const WindowSession& candidate) {
                return candidate.renderSurface.nativeWindowHandle == handle;
            });
        if (session == App().windowSessions.end()) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application,
                NeoWPF::Diagnostics::Level::Debug,
                "[Surface] release requested for unknown window handle=%p", handle);
            return false;
        }
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application,
            NeoWPF::Diagnostics::Level::Info,
            "[Surface] release begin: handle=%p host=%p generation=%llu valid=%d",
            handle, session->windowHost.get(),
            static_cast<unsigned long long>(session->renderSurface.GetGenerationId()),
            session->renderSurface.IsValid() ? 1 : 0);
        wait = QueueDestroyLocked(*session);
    }

    WaitDestroy(wait);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info, "[Surface] release complete: handle=%p", handle);
    return true;
}

bool AttachWindowSurface(void* handle)
{
    if (!handle) {
        return false;
    }

    std::lock_guard<std::mutex> lock(App().windowSessionsMutex);
    const auto session = std::find_if(
        App().windowSessions.begin(), App().windowSessions.end(),
        [handle](const WindowSession& candidate) {
            return candidate.renderSurface.nativeWindowHandle == handle;
        });
    if (session == App().windowSessions.end()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application,
            NeoWPF::Diagnostics::Level::Debug,
            "[Surface] attach requested for unknown window handle=%p", handle);
        return false;
    }
    if (session->lifecycle.IsShutdown() || session->renderSurface.IsValid()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application,
            NeoWPF::Diagnostics::Level::Debug,
            "[Surface] attach rejected: handle=%p host=%p shutdown=%d valid=%d generation=%llu",
            handle, session->windowHost.get(), session->lifecycle.IsShutdown() ? 1 : 0,
            session->renderSurface.IsValid() ? 1 : 0,
            static_cast<unsigned long long>(session->renderSurface.GetGenerationId()));
        return false;
    }

    session->renderSurface = NeoWPF::Rendering::RenderSurface{};
    session->renderSurface.nativeWindowHandle = handle;
    session->width = 0;
    session->height = 0;
    session->needsLayout = true;
    session->commandListInitialized = false;
    const bool activated = session->lifecycle.Activate();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application,
        NeoWPF::Diagnostics::Level::Info,
        "[Surface] attach %s: handle=%p host=%p generation=%llu",
        activated ? "complete" : "failed", handle, session->windowHost.get(),
        static_cast<unsigned long long>(session->renderSurface.GetGenerationId()));
    return activated;
}

bool CloseWindow(void* handle)
{
    NEO_WPF_ACTION("Application.CloseWindow");

    if (!handle) {
        return false;
    }

    std::shared_ptr<NeoWPF::UI::Window> windowHost;
    DestroyWait destroyWait;
    bool removed = false;
    {
        std::lock_guard<std::mutex> lock(App().windowSessionsMutex);
        auto it = std::find_if(App().windowSessions.begin(), App().windowSessions.end(), [handle](const WindowSession& windowSession) {
            return windowSession.renderSurface.nativeWindowHandle == handle;
        });
        if (it != App().windowSessions.end()) {
            windowHost = it->windowHost;
            it->lifecycle.Shutdown();
            destroyWait = QueueDestroyLocked(*it);
            App().windowSessions.erase(it);
            removed = true;
        }
    }

    if (!removed) {
        return false;
    }

    WaitDestroy(destroyWait);

    if (windowHost) {
        NeoWPF::Input::InputManager::GetInstance().NotifyVisualTreeReplaced(windowHost.get());
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "CloseWindow");

    return true;
}

}
