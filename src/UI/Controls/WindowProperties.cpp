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

NeoWPF::UI::Properties::PropertyMetadata MakeWindowBackgroundMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu));
    metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

NeoWPF::UI::Properties::PropertyMetadata MakeWindowTransparencyMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(false);
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

const NeoWPF::UI::Properties::DependencyProperty& Window::LocalesProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<
        Window,
        std::vector<std::shared_ptr<NeoWPF::UI::Localization::LocaleTable>>>("Locales");
    static const bool bindingRegistered = (NeoWPF::UI::Markup::Xaml::RegisterXamlCollectionProperty(
        property,
        &Window::AddLocale), true);
    (void)bindingRegistered;
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& Window::BackgroundColorProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<
        Window, NeoWPF::UI::Color>(
            "Background",
            &Window::GetBackgroundColor,
            static_cast<void (Window::*)(NeoWPF::UI::Color)>(&Window::SetBackgroundColor),
            MakeWindowBackgroundMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& Window::AllowsTransparencyProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<
        Window, bool>(
            "AllowsTransparency",
            &Window::IsTransparent,
            &Window::SetAllowsTransparency,
            MakeWindowTransparencyMetadata());
    return property;
}

}

namespace NeoWPF::UI::Properties {

void RegisterWindowProperties()
{
    (void)NeoWPF::UI::Window::ContentProperty();
    (void)NeoWPF::UI::Window::TitleBarProperty();
    (void)NeoWPF::UI::Window::TitleProperty();
    (void)NeoWPF::UI::Window::LocalesProperty();
    (void)NeoWPF::UI::Window::BackgroundColorProperty();
    (void)NeoWPF::UI::Window::AllowsTransparencyProperty();
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<NeoWPF::UI::Window>(
        "Window",
        "Content",
        "NeoWPF::UI::Window",
        "NeoWPF/UI/Controls/Window.h",
        NeoWPF::UI::Markup::Xaml::XamlTypeRole::WindowRoot);
}

}
