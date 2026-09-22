#include <NeoWPF/UI/Controls/RadioButton/RadioButton.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterRadioButtonProperties()
{
    PropertyMetadata text;
    text.DefaultValue = std::any(std::wstring());
    text.Converter = GetWideStringConverter();
    text.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
    RegisterProperty<UI::RadioButton, std::wstring>("Text", &UI::RadioButton::GetText, &UI::RadioButton::SetText, text);

    auto colorMetadata = [](NeoWPF::UI::Color color) {
        PropertyMetadata metadata;
        metadata.DefaultValue = std::any(color);
        metadata.Converter = GetColorConverter();
        metadata.Flags = PropertyFlags::AffectsRender;
        return metadata;
    };
    RegisterProperty<UI::RadioButton, UI::Color>("Foreground", &UI::RadioButton::GetTextColor, &UI::RadioButton::SetTextColor, colorMetadata(UI::Color::FromArgb(0xFF000000u)));
    RegisterProperty<UI::RadioButton, UI::Color>("Background", &UI::RadioButton::GetBackgroundColor, &UI::RadioButton::SetBackgroundColor, colorMetadata(UI::Color::FromArgb(0xFFFFFFFFu)));
    RegisterProperty<UI::RadioButton, UI::Color>("BorderBrush", &UI::RadioButton::GetBorderColor, &UI::RadioButton::SetBorderColor, colorMetadata(UI::Color::FromArgb(0xFF202020u)));

    PropertyMetadata padding;
    padding.DefaultValue = std::any(UI::Thickness(8));
    padding.Converter = GetThicknessConverter();
    padding.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
    RegisterProperty<UI::RadioButton, UI::Thickness>("Padding", &UI::RadioButton::GetPadding, &UI::RadioButton::SetPadding, padding);

    PropertyMetadata fontSize;
    fontSize.DefaultValue = std::any(14);
    fontSize.Converter = GetIntConverter();
    fontSize.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
    RegisterProperty<UI::RadioButton, int>("FontSize", &UI::RadioButton::GetFontSize, &UI::RadioButton::SetFontSize, fontSize);

    PropertyMetadata fontFamily;
    fontFamily.DefaultValue = std::any(std::wstring());
    fontFamily.Converter = GetWideStringConverter();
    fontFamily.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
    RegisterProperty<UI::RadioButton, std::wstring>("FontFamily", &UI::RadioButton::GetFontFamily, &UI::RadioButton::SetFontFamily, fontFamily);

    PropertyMetadata fontWeight;
    fontWeight.DefaultValue = std::any(400);
    fontWeight.Converter = GetFontWeightConverter();
    fontWeight.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
    RegisterProperty<UI::RadioButton, int>("FontWeight", &UI::RadioButton::GetFontWeight, &UI::RadioButton::SetFontWeight, fontWeight);

    PropertyMetadata fontStyle;
    fontStyle.DefaultValue = std::any(UI::FontStyle::Normal);
    fontStyle.Converter = GetFontStyleConverter();
    fontStyle.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
    RegisterProperty<UI::RadioButton, UI::FontStyle>("FontStyle", &UI::RadioButton::GetFontStyle, &UI::RadioButton::SetFontStyle, fontStyle);

    PropertyMetadata checked;
    checked.DefaultValue = std::any(false);
    checked.Converter = GetBoolConverter();
    RegisterProperty<UI::RadioButton, bool>("IsChecked", &UI::RadioButton::IsChecked, &UI::RadioButton::SetChecked, checked);

    PropertyMetadata group;
    group.DefaultValue = std::any(std::wstring());
    group.Converter = GetWideStringConverter();
    RegisterProperty<UI::RadioButton, std::wstring>("GroupName", &UI::RadioButton::GetGroupName, &UI::RadioButton::SetGroupName, group);

    ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::RadioButton>("RadioButton", "", "NeoWPF::UI::RadioButton", "NeoWPF/UI/Controls/RadioButton/RadioButton.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::TextProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, std::wstring>(
        "Text", &NeoWPF::UI::RadioButton::GetText, &NeoWPF::UI::RadioButton::SetText);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::TextColorProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, NeoWPF::UI::Color>(
        "Foreground", &NeoWPF::UI::RadioButton::GetTextColor, &NeoWPF::UI::RadioButton::SetTextColor);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::BackgroundColorProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, NeoWPF::UI::Color>(
        "Background", &NeoWPF::UI::RadioButton::GetBackgroundColor, &NeoWPF::UI::RadioButton::SetBackgroundColor);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::BorderColorProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, NeoWPF::UI::Color>(
        "BorderBrush", &NeoWPF::UI::RadioButton::GetBorderColor, &NeoWPF::UI::RadioButton::SetBorderColor);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::PaddingProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, NeoWPF::UI::Thickness>(
        "Padding", &NeoWPF::UI::RadioButton::GetPadding, &NeoWPF::UI::RadioButton::SetPadding);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::FontSizeProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, int>(
        "FontSize", &NeoWPF::UI::RadioButton::GetFontSize, &NeoWPF::UI::RadioButton::SetFontSize);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::FontFamilyProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, std::wstring>(
        "FontFamily", &NeoWPF::UI::RadioButton::GetFontFamily, &NeoWPF::UI::RadioButton::SetFontFamily);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::FontWeightProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, int>(
        "FontWeight", &NeoWPF::UI::RadioButton::GetFontWeight, &NeoWPF::UI::RadioButton::SetFontWeight);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::FontStyleProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, NeoWPF::UI::FontStyle>(
        "FontStyle", &NeoWPF::UI::RadioButton::GetFontStyle, &NeoWPF::UI::RadioButton::SetFontStyle);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::IsCheckedProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, bool>(
        "IsChecked", &NeoWPF::UI::RadioButton::IsChecked, &NeoWPF::UI::RadioButton::SetChecked);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RadioButton::GroupNameProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RadioButton, std::wstring>(
        "GroupName", &NeoWPF::UI::RadioButton::GetGroupName, &NeoWPF::UI::RadioButton::SetGroupName);
    return *property;
}