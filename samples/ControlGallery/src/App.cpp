#include <NeoWPF/NeoWPF.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/CrashHandler.h>
#include <Gallery/MainWindow.h>

int main()
{
#ifndef NDEBUG
    using NeoWPF::Diagnostics::Category;
    using NeoWPF::Diagnostics::Level;

    NeoWPF::Diagnostics::SetGlobalLevel(Level::Trace);

    NeoWPF::Diagnostics::SetLevel(Category::GridLayout,       Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::Control,          Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::HitTest,          Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::Virtualization,   Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::TextBox,          Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::Image,            Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::Http,             Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::XamlParser,       Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::ListBox,          Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::ComboBox,         Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::Popup,            Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::ItemsControl,     Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::Clipboard,        Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::Files,            Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::Input,            Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::Visual,        Level::Off);
    NeoWPF::Diagnostics::SetLevel(Category::TextRendering, Level::Trace);
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
