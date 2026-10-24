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

}

bool CloseApplicationWindow(void* nativeWindowHandle)
{
    NEO_WPF_ACTION("Application.CloseWindow");

    if (!nativeWindowHandle) {
        return false;
    }

    std::shared_ptr<NeoWPF::UI::Window> windowHost;
    std::shared_ptr<RenderSurfaceDestructionRequest> destructionRequest;
    bool removed = false;
    bool rendererInitialized = false;
    bool waitForRendererShutdown = false;
    {
        std::lock_guard<std::mutex> lock(App().windowSessionsMutex);
        auto it = std::find_if(App().windowSessions.begin(), App().windowSessions.end(), [nativeWindowHandle](const WindowSession& windowSession) {
            return windowSession.renderSurface.nativeWindowHandle == nativeWindowHandle;
        });
        if (it != App().windowSessions.end()) {
            windowHost = it->windowHost;
            it->renderSurface.Invalidate();
            if (App().renderThreadInitialized.load()) {
                waitForRendererShutdown = App().renderThreadShuttingDown;
                rendererInitialized = !waitForRendererShutdown;
            }
            if (rendererInitialized) {
                destructionRequest = std::make_shared<RenderSurfaceDestructionRequest>();
                destructionRequest->surface = it->renderSurface;
                App().pendingRenderSurfaceDestruction.push_back(destructionRequest);
            }
            App().windowSessions.erase(it);
            removed = true;
        }
    }

    if (!removed) {
        return false;
    }

    if (rendererInitialized) {
        {
            std::lock_guard<std::mutex> lock(App().renderRequestMutex);
            App().renderRequested = true;
        }
        App().renderRequestCondition.notify_one();
        std::unique_lock<std::mutex> lock(App().windowSessionsMutex);
        App().renderSurfaceDestructionCondition.wait(lock, [&destructionRequest] {
            return destructionRequest->completed;
        });
    } else if (waitForRendererShutdown) {
        std::unique_lock<std::mutex> lock(App().windowSessionsMutex);
        App().renderSurfaceDestructionCondition.wait(lock, [] {
            return !App().renderThreadInitialized.load();
        });
    }

    if (windowHost) {
        NeoWPF::Input::InputManager::GetInstance().NotifyVisualTreeReplaced(windowHost.get());
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "CloseApplicationWindow");

    return true;
}

}
