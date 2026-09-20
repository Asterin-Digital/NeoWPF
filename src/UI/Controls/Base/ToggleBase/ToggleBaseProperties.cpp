#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterToggleBaseProperties()
{
    PropertyMetadata checkedMetadata;
    checkedMetadata.DefaultValue = std::any(false);
    checkedMetadata.Converter = GetBoolConverter();
    RegisterProperty<UI::ToggleBase, bool>(
        "IsChecked", &UI::ToggleBase::IsChecked, &UI::ToggleBase::SetChecked, checkedMetadata);

    PropertyMetadata contentTextMetadata;
    contentTextMetadata.DefaultValue = std::any(std::wstring());
    contentTextMetadata.Converter = GetWideStringConverter();
    contentTextMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
    RegisterProperty<UI::ToggleBase, std::wstring>(
        "ContentText", &UI::ToggleBase::GetContentText, &UI::ToggleBase::SetContentText, contentTextMetadata);
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ToggleBase::IsCheckedProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ToggleBase, bool>(
            "IsChecked", &NeoWPF::UI::ToggleBase::IsChecked, &NeoWPF::UI::ToggleBase::SetChecked);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ToggleBase::ContentTextProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ToggleBase, std::wstring>(
            "ContentText", &NeoWPF::UI::ToggleBase::GetContentText,
            &NeoWPF::UI::ToggleBase::SetContentText);
    return *property;
}