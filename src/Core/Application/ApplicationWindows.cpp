#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/Window.h>
#include <mutex>

namespace NeoWPF::Core {
namespace {

AppState& App() { return GetAppState(); }

} // namespace

HWND CreateApplicationWindow(const wchar_t* title, int width, int height)
{
    if (!App().hInst) {
        App().hInst = GetModuleHandleW(nullptr);
    }

    HWND hwnd = NeoWPF::Platform::Window::CreateWindow(App().hInst, title, width, height);
    if (!hwnd) {
        return nullptr;
    }

    WindowState windowState;
    windowState.hwnd = hwnd;
    windowState.width = width;
    windowState.height = height;
    windowState.needsLayout = true;

    {
        std::lock_guard<std::mutex> lock(App().windowsMutex);
        App().windows.push_back(windowState);
    }

    return hwnd;
}

}
