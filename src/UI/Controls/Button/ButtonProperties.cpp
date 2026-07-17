#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterButtonProperties() {
        PropertyMetadata textMetadata;
        textMetadata.DefaultValue = std::any(std::wstring());
        textMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::Button, std::wstring>(
            "Text",
            &UI::Button::GetText,
            &UI::Button::SetText,
            textMetadata);

        PropertyMetadata textColorMetadata;
        textColorMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFF000000u));
        textColorMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::Button, NeoWPF::UI::Color>(
            "TextColor",
            &UI::Button::GetTextColor,
            static_cast<void (UI::Button::*)(NeoWPF::UI::Color)>(&UI::Button::SetTextColor),
            textColorMetadata);

        PropertyMetadata fontSizeMetadata;
        fontSizeMetadata.DefaultValue = std::any(16);
        fontSizeMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::Button, int>(
            "FontSize",
            &UI::Button::GetFontSize,
            &UI::Button::SetFontSize,
            fontSizeMetadata);

        PropertyMetadata paddingMetadata;
        paddingMetadata.DefaultValue = std::any(8);
        paddingMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
        RegisterProperty<UI::Button, int>(
            "Padding",
            &UI::Button::GetPadding,
            &UI::Button::SetPadding,
            paddingMetadata);

        PropertyMetadata bgColorMetadata;
        bgColorMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFF2F2F2Fu));
        bgColorMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::Button, NeoWPF::UI::Color>(
            "BackgroundColor",
            &UI::Button::GetBackgroundColor,
            static_cast<void (UI::Button::*)(NeoWPF::UI::Color)>(&UI::Button::SetBackgroundColor),
            bgColorMetadata);

        PropertyMetadata borderColorMetadata;
        borderColorMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFF333333u));
        borderColorMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::Button, NeoWPF::UI::Color>(
            "BorderColor",
            &UI::Button::GetBorderColor,
            static_cast<void (UI::Button::*)(NeoWPF::UI::Color)>(&UI::Button::SetBorderColor),
            borderColorMetadata);
    }
}
