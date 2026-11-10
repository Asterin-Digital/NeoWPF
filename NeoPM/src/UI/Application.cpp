#include <NeoPM/UI/Application.h>
#include <NeoPM/UI/Components/Theme.h>
#include <NeoPM/UI/Navigation/Navigation.h>
#include <NeoPM/UI/Resources/Font.h>
#include <NeoPM/Utils/Logger.h>

#include <NeoWPF/NeoWPF.h>

namespace NeoPM::UI {

int Run()
{
    NeoWPF::Core::Application::Initialize();
    if (!NeoWPF::Core::Application::LoadFontFromMemory(
        const auto fontResult = NeoWPF::Core::Application::LoadFontFromMemory(
            Resources::FontData,
            Resources::FontDataSize,
            Theme::FontFamily);
        if (fontResult.Fail()) {
        NeoPM::Utils::Logger::Instance().Error(
            "GuiApplication", "Failed to register embedded Noto Sans font");
    }
    NeoWPF::Core::Application::SetResource(
        "FontFamily", String(Theme::FontFamily));
    Theme::Register();
    auto& application = NeoWPF::Core::Application::Current();
    auto windowHost = New<NeoWPF::UI::Window>();
    windowHost->SetTitle(L"NeoPM");
    windowHost->SetBackgroundBrush(New<NeoWPF::UI::SolidColorBrush>(Theme::WindowBackground));

    auto window = application.CreateWindow(windowHost, 960, 640);
    if (!window.Handle()) {
        NeoPM::Utils::Logger::Instance().Error(
            "GuiApplication", "Failed to create the NeoPM main window");
        NeoWPF::Core::Application::Shutdown();
        return -1;
    }

    window.SetContent(Navigation::CreateMainContent());

    const auto result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    return result;
}

}
