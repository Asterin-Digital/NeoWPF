#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <algorithm>
#include <mutex>

namespace NeoWPF::Core {
namespace {

AppState& App() { return GetAppState(); }

}

void* CreateApplicationWindow(const wchar_t* title, int width, int height)
{
    if (!App().hInst) {
        App().hInst = GetModuleHandleW(nullptr);
    }

    void* nativeHandle = NeoWPF::Platform::Window::CreateWindow(App().hInst, title, width, height);
    if (!nativeHandle) {
        NeoWPF::Diagnostics::LogError("CreateApplicationWindow failed for title=%ls width=%d height=%d", title, width, height);
        return nullptr;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "CreateApplicationWindow created title=%ls", title);

    WindowState windowState;
    windowState.nativeWindowHandle = nativeHandle;
    windowState.width = width;
    windowState.height = height;
    windowState.needsLayout = true;
    windowState.isPrimaryWindow = App().windows.empty();
    windowState.windowHost = std::make_shared<NeoWPF::UI::Window>();
    windowState.contentRoot = nullptr;

    {
        std::lock_guard<std::mutex> lock(App().windowsMutex);
        App().windows.push_back(windowState);
    }

    HWND hwnd = static_cast<HWND>(nativeHandle);
    ::ShowWindow(hwnd, SW_SHOWNORMAL);
    ::UpdateWindow(hwnd);
    ::RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);

    return nativeHandle;
}

bool CloseApplicationWindow(void* nativeWindowHandle)
{
    if (!nativeWindowHandle) {
        return false;
    }

    HWND hwnd = static_cast<HWND>(nativeWindowHandle);

    NeoWPF::Input::InputManager::GetInstance().SetRootElement(nullptr);

    {
        std::lock_guard<std::mutex> lock(App().windowsMutex);
        auto it = std::find_if(App().windows.begin(), App().windows.end(), [hwnd](const WindowState& windowState) {
            return windowState.nativeWindowHandle == hwnd;
        });
        if (it != App().windows.end()) {
            App().windows.erase(it);
        }
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "CloseApplicationWindow");

    if (IsWindow(hwnd)) {
        return ::DestroyWindow(hwnd) != FALSE;
    }

    return true;
}

}
