#pragma once

#include <Windows.h>
#include <vector>

// Undef Windows macro to avoid conflict with CreateWindow method
#undef CreateWindow

namespace NeoWPF::UI {
class FrameworkElement;
class Panel;
}

namespace NeoWPF::Core {

class Application {
public:
    static void Initialize();
    static int Run();
    static void Shutdown();
    static Application* Current();

    // Create a new window managed by this application
    HWND CreateWindow(const wchar_t* title = L"NeoWPF", int width = 800, int height = 600);

private:
    Application() = default;
    static Application* s_current;
};

}
