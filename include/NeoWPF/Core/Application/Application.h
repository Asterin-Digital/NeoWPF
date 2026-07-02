#pragma once

#include <Windows.h>
#include <vector>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>

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

    // Dump the visual tree from all root elements for debugging
    // Use VisualTreeDumpOptions to control output detail level and depth
    static void DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options = NeoWPF::Diagnostics::VisualTreeDumpOptions{});

private:
    Application() = default;
    static Application* s_current;
};

}
