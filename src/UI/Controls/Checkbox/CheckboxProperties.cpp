#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterCheckboxProperties() {
        PropertyMetadata textMetadata;
        textMetadata.DefaultValue = std::any(std::wstring());
        textMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::Checkbox, std::wstring>(
            "Text",
            &UI::Checkbox::GetText,
            &UI::Checkbox::SetText,
            textMetadata);

        PropertyMetadata textColorMetadata;
        textColorMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFFEDEDEDu));
        textColorMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::Checkbox, NeoWPF::UI::Color>(
            "TextColor",
            &UI::Checkbox::GetTextColor,
            static_cast<void (UI::Checkbox::*)(NeoWPF::UI::Color)>(&UI::Checkbox::SetTextColor),
            textColorMetadata);

        PropertyMetadata backgroundMetadata;
        backgroundMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFF3A3A3Au));
        backgroundMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::Checkbox, NeoWPF::UI::Color>(
            "BackgroundColor",
            &UI::Checkbox::GetBackgroundColor,
            static_cast<void (UI::Checkbox::*)(NeoWPF::UI::Color)>(&UI::Checkbox::SetBackgroundColor),
            backgroundMetadata);

        PropertyMetadata borderMetadata;
        borderMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFF666666u));
        borderMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::Checkbox, NeoWPF::UI::Color>(
            "BorderColor",
            &UI::Checkbox::GetBorderColor,
            static_cast<void (UI::Checkbox::*)(NeoWPF::UI::Color)>(&UI::Checkbox::SetBorderColor),
            borderMetadata);

        PropertyMetadata paddingMetadata;
        paddingMetadata.DefaultValue = std::any(8);
        paddingMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
        RegisterProperty<UI::Checkbox, int>(
            "Padding",
            &UI::Checkbox::GetPadding,
            &UI::Checkbox::SetPadding,
            paddingMetadata);

        PropertyMetadata fontSizeMetadata;
        fontSizeMetadata.DefaultValue = std::any(14);
        fontSizeMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::Checkbox, int>(
            "FontSize",
            &UI::Checkbox::GetFontSize,
            &UI::Checkbox::SetFontSize,
            fontSizeMetadata);
    }
}
