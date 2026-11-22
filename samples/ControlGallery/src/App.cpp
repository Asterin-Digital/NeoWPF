#include <NeoWPF/NeoWPF.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/CrashHandler.h>
#include <Gallery/MainWindow.h>

int main()
{
#ifndef NDEBUG
    NeoWPF::Diagnostics::SetGlobalLevel(NeoWPF::Diagnostics::Level::Trace);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Off);
#endif
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::InitializeCrashHandler();

    auto windowHost = Gallery::CreateMainWindow();
    NeoWPF::Core::Application::Current().CreateWindow(windowHost, 720, 420);

    const auto result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    NeoWPF::Diagnostics::ShutdownCrashHandler();
    return result;
}
