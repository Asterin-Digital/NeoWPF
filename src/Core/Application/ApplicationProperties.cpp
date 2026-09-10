#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>

namespace NeoWPF::Core {

const NeoWPF::UI::Properties::DependencyProperty& Application::MainWindowProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<
        Application, std::shared_ptr<NeoWPF::UI::Window>>(
            "MainWindow", &Application::GetMainWindow, &Application::SetMainWindow);
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& Application::LocalesProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<
        Application,
        std::vector<std::shared_ptr<NeoWPF::UI::Localization::LocaleTable>>>("Locales");
    static const bool bindingRegistered = (NeoWPF::UI::Markup::Xaml::RegisterXamlCollectionProperty(
        property,
        &Application::AddLocale), true);
    (void)bindingRegistered;
    return property;
}

}

namespace NeoWPF::UI::Properties {

void RegisterApplicationProperties()
{
    (void)NeoWPF::Core::Application::MainWindowProperty();
    (void)NeoWPF::Core::Application::LocalesProperty();
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<NeoWPF::Core::Application>(
        "Application",
        "",
        "NeoWPF::Core::Application",
        "NeoWPF/Core/Application/Application.h",
        NeoWPF::UI::Markup::Xaml::XamlTypeRole::ApplicationRoot);
}

}
