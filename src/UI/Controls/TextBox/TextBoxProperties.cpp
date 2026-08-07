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
            static_cast<void (UI::TextBox::*)(const std::wstring&)>(&UI::TextBox::SetText),
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

        PropertyMetadata fontFamilyMetadata;
        fontFamilyMetadata.DefaultValue = std::any(std::wstring());
        fontFamilyMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBox, std::wstring>(
            "FontFamily",
            &UI::TextBox::GetFontFamily,
            &UI::TextBox::SetFontFamily,
            fontFamilyMetadata);

        PropertyMetadata textHorizontalAlignmentMetadata;
        textHorizontalAlignmentMetadata.DefaultValue = std::any(TextHorizontalAlignment::Left);
        textHorizontalAlignmentMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBox, TextHorizontalAlignment>(
            "TextHorizontalAlignment",
            &UI::TextBox::GetTextHorizontalAlignment,
            static_cast<void (UI::TextBox::*)(TextHorizontalAlignment)>(&UI::TextBox::SetTextHorizontalAlignment),
            textHorizontalAlignmentMetadata);

        PropertyMetadata textVerticalAlignmentMetadata;
        textVerticalAlignmentMetadata.DefaultValue = std::any(TextVerticalAlignment::Top);
        textVerticalAlignmentMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBox, TextVerticalAlignment>(
            "TextVerticalAlignment",
            &UI::TextBox::GetTextVerticalAlignment,
            static_cast<void (UI::TextBox::*)(TextVerticalAlignment)>(&UI::TextBox::SetTextVerticalAlignment),
            textVerticalAlignmentMetadata);

    }
}
