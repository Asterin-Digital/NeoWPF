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
    bool removed = false;
    {
        std::lock_guard<std::mutex> lock(App().windowSessionsMutex);
        auto it = std::find_if(App().windowSessions.begin(), App().windowSessions.end(), [nativeWindowHandle](const WindowSession& windowSession) {
            return windowSession.nativeWindowHandle == nativeWindowHandle;
        });
        if (it != App().windowSessions.end()) {
            windowHost = it->windowHost;
            App().windowSessions.erase(it);
            removed = true;
        }
    }

    if (!removed) {
        return false;
    }

    if (windowHost) {
        NeoWPF::Input::InputManager::GetInstance().NotifyVisualTreeReplaced(windowHost.get());
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "CloseApplicationWindow");

    return true;
}

}
