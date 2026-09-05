#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {

NeoWPF::UI::Properties::PropertyMetadata MakeWindowTitleMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(std::wstring(L"NeoWPF"));
    metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
    return metadata;
}

}

namespace NeoWPF::UI {

const NeoWPF::UI::Properties::DependencyProperty& Window::ContentProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<
        Window, std::shared_ptr<Visual>>("Content");
    static const bool bindingRegistered = (NeoWPF::UI::Markup::Xaml::RegisterXamlElementProperty<
        Window, Visual>(property, &Window::SetContent), true);
    (void)bindingRegistered;
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& Window::TitleBarProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<
        Window, std::shared_ptr<Visual>>("TitleBar");
    static const bool bindingRegistered = (NeoWPF::UI::Markup::Xaml::RegisterXamlElementProperty<
        Window, Visual>(property, &Window::SetTitleBar), true);
    (void)bindingRegistered;
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& Window::TitleProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<
        Window, std::wstring>(
            "Title",
            &Window::GetTitleValue,
            static_cast<void (Window::*)(const std::wstring&)>(&Window::SetTitle),
            MakeWindowTitleMetadata());
    return property;
}

}

namespace NeoWPF::UI::Properties {

void RegisterWindowProperties()
{
    (void)NeoWPF::UI::Window::ContentProperty();
    (void)NeoWPF::UI::Window::TitleBarProperty();
    (void)NeoWPF::UI::Window::TitleProperty();
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<NeoWPF::UI::Window>(
        "Window",
        "Content",
        "NeoWPF::UI::Window",
        "NeoWPF/UI/Controls/Window.h",
        NeoWPF::UI::Markup::Xaml::XamlTypeRole::WindowRoot);
}

}
