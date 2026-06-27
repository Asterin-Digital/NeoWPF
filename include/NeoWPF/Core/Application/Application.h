#pragma once

namespace NeoWPF::UI {
class FrameworkElement;
class Panel;
}

namespace NeoWPF::Core {

class Application {
public:
    static void Initialize();
    static int Run(const wchar_t* title = L"NeoWPF", int width = 800, int height = 600);
    static void Shutdown();
};

}
