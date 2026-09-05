#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::Core {

const NeoWPF::UI::Properties::DependencyProperty& Application::MainWindowProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<
        Application, std::shared_ptr<NeoWPF::UI::Window>>(
            "MainWindow", &Application::GetMainWindow, &Application::SetMainWindow);
    return property;
}

}

namespace NeoWPF::UI::Properties {

void RegisterApplicationProperties()
{
    (void)NeoWPF::Core::Application::MainWindowProperty();
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<NeoWPF::Core::Application>(
        "Application",
        "",
        "NeoWPF::Core::Application",
        "NeoWPF/Core/Application/Application.h",
        NeoWPF::UI::Markup::Xaml::XamlTypeRole::ApplicationRoot);
}

}
