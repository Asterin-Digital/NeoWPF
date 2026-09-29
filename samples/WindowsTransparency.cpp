#include <NeoWPF/NeoWPF.h>
#include <memory>

int main()
{
    NeoWPF::Core::Application::Initialize();

    auto host = std::make_shared<NeoWPF::UI::Window>();
    host->SetTitle(L"NeoWPF Windows Transparency");
    host->SetAllowsTransparency(true);
    host->SetBackgroundColor(0x00000000u);

    auto content = std::make_shared<NeoWPF::UI::Border>();
    content->SetWidth(280.0);
    content->SetHeight(180.0);
    content->SetBackgroundColor(0xCC2463A5u);
    host->SetContent(content);

    auto window = NeoWPF::Core::Application::Current().CreateWindow(host, 640, 420);
    if (!window.Handle()) {
        NeoWPF::Core::Application::Shutdown();
        return 1;
    }

    auto opaqueWindow = NeoWPF::Core::Application::Current().CreateWindow(
        L"NeoWPF Opaque Window", 420, 280);
    if (!opaqueWindow.Handle()) {
        NeoWPF::Core::Application::Shutdown();
        return 1;
    }

    const int result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    return result;
}
