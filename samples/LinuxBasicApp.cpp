#include <NeoWPF/NeoWPF.h>

int main()
{
    NeoWPF::Core::Application::Initialize();
    auto window = NeoWPF::Core::Application::Current().CreateWindow(L"NeoWPF Linux Vulkan", 800, 480);
    if (!window.Handle()) {
        return -1;
    }

    const int result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    return result;
}
