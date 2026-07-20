#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterTextBoxProperties() {
        PropertyMetadata textMetadata;
        textMetadata.DefaultValue = std::any(std::wstring());
        textMetadata.Flags = PropertyFlags::None;
        RegisterProperty<UI::TextBox, std::wstring>(
            "Text",
            &UI::TextBox::GetText,
            &UI::TextBox::SetText,
            textMetadata);

        PropertyMetadata textColorMetadata;
        textColorMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFFEDEDEDu));
        textColorMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBox, NeoWPF::UI::Color>(
            "TextColor",
            &UI::TextBox::GetTextColor,
            static_cast<void (UI::TextBox::*)(NeoWPF::UI::Color)>(&UI::TextBox::SetTextColor),
            textColorMetadata);

        PropertyMetadata caretMetadata;
        caretMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu));
        caretMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBox, NeoWPF::UI::Color>(
            "CaretColor",
            &UI::TextBox::GetCaretColor,
            static_cast<void (UI::TextBox::*)(NeoWPF::UI::Color)>(&UI::TextBox::SetCaretColor),
            caretMetadata);

        PropertyMetadata fontSizeMetadata;
        fontSizeMetadata.DefaultValue = std::any(16);
        fontSizeMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBox, int>(
            "FontSize",
            &UI::TextBox::GetFontSize,
            &UI::TextBox::SetFontSize,
            fontSizeMetadata);

    }
}
