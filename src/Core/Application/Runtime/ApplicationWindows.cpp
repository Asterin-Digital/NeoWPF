#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/Logger.h>
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
    if (!nativeWindowHandle) {
        return false;
    }

    NeoWPF::Input::InputManager::GetInstance().SetRootElement(nullptr);

    {
        std::lock_guard<std::mutex> lock(App().windowsMutex);
        auto it = std::find_if(App().windows.begin(), App().windows.end(), [nativeWindowHandle](const WindowState& windowState) {
            return windowState.nativeWindowHandle == nativeWindowHandle;
        });
        if (it != App().windows.end()) {
            App().windows.erase(it);
        }
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "CloseApplicationWindow");

    return true;
}

}
