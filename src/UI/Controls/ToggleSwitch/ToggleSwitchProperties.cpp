#include <NeoWPF/UI/Controls/ToggleSwitch/ToggleSwitch.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterToggleSwitchProperties()
{
    PropertyMetadata checkedMetadata;
    checkedMetadata.DefaultValue = std::any(false);
    checkedMetadata.Converter = GetBoolConverter();
    RegisterProperty<UI::ToggleSwitch, bool>(
        "IsChecked", &UI::ToggleSwitch::IsChecked, &UI::ToggleSwitch::SetChecked,
        checkedMetadata);

    PropertyMetadata contentTextMetadata;
    contentTextMetadata.DefaultValue = std::any(std::wstring());
    contentTextMetadata.Converter = GetWideStringConverter();
    contentTextMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
    RegisterProperty<UI::ToggleSwitch, std::wstring>(
        "ContentText", &UI::ToggleSwitch::GetContentText, &UI::ToggleSwitch::SetContentText,
        contentTextMetadata);

    ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::ToggleSwitch>(
        "ToggleSwitch", "", "NeoWPF::UI::ToggleSwitch",
        "NeoWPF/UI/Controls/ToggleSwitch/ToggleSwitch.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ToggleSwitch::IsCheckedProperty()
{
    static const auto& property =
        NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::ToggleSwitch, bool>(
            "IsChecked", &NeoWPF::UI::ToggleSwitch::IsChecked,
            &NeoWPF::UI::ToggleSwitch::SetChecked);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ToggleSwitch::ContentTextProperty()
{
    static const auto& property =
        NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::ToggleSwitch, std::wstring>(
            "ContentText", &NeoWPF::UI::ToggleSwitch::GetContentText,
            &NeoWPF::UI::ToggleSwitch::SetContentText);
    return *property;
}