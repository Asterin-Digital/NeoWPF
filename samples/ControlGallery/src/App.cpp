#include <NeoWPF/NeoWPF.h>
#include <Gallery/MainWindow.h>

int main()
{
    NeoWPF::Core::Application::Initialize();

    auto windowHost = Gallery::CreateMainWindow();
    NeoWPF::Core::Application::Current().CreateWindow(windowHost, 720, 420);

    const auto result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    return result;
}
