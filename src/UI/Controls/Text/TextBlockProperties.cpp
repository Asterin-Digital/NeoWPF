#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterTextBlockProperties() {
        PropertyMetadata textMetadata;
        textMetadata.DefaultValue = std::any(std::wstring());
        textMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBlock, std::wstring>(
            "Text",
            &UI::TextBlock::GetText,
            &UI::TextBlock::SetText,
            textMetadata);

        PropertyMetadata textColorMetadata;
        textColorMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFF000000u));
        textColorMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBlock, NeoWPF::UI::Color>(
            "TextColor",
            &UI::TextBlock::GetTextColor,
            static_cast<void (UI::TextBlock::*)(NeoWPF::UI::Color)>(&UI::TextBlock::SetTextColor),
            textColorMetadata);

        PropertyMetadata fontSizeMetadata;
        fontSizeMetadata.DefaultValue = std::any(16);
        fontSizeMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBlock, int>(
            "FontSize",
            &UI::TextBlock::GetFontSize,
            &UI::TextBlock::SetFontSize,
            fontSizeMetadata);

        PropertyMetadata fontFamilyMetadata;
        fontFamilyMetadata.DefaultValue = std::any(std::wstring());
        fontFamilyMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBlock, std::wstring>(
            "FontFamily",
            &UI::TextBlock::GetFontFamily,
            &UI::TextBlock::SetFontFamily,
            fontFamilyMetadata);

        PropertyMetadata textHorizontalAlignmentMetadata;
        textHorizontalAlignmentMetadata.DefaultValue = std::any(TextHorizontalAlignment::Left);
        textHorizontalAlignmentMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBlock, TextHorizontalAlignment>(
            "TextHorizontalAlignment",
            &UI::TextBlock::GetTextHorizontalAlignment,
            static_cast<void (UI::TextBlock::*)(TextHorizontalAlignment)>(&UI::TextBlock::SetTextHorizontalAlignment),
            textHorizontalAlignmentMetadata);

        PropertyMetadata textVerticalAlignmentMetadata;
        textVerticalAlignmentMetadata.DefaultValue = std::any(TextVerticalAlignment::Top);
        textVerticalAlignmentMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBlock, TextVerticalAlignment>(
            "TextVerticalAlignment",
            &UI::TextBlock::GetTextVerticalAlignment,
            static_cast<void (UI::TextBlock::*)(TextVerticalAlignment)>(&UI::TextBlock::SetTextVerticalAlignment),
            textVerticalAlignmentMetadata);

        PropertyMetadata textWrappingMetadata;
        textWrappingMetadata.DefaultValue = std::any(TextWrapping::NoWrap);
        textWrappingMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBlock, TextWrapping>(
            "TextWrapping",
            &UI::TextBlock::GetTextWrapping,
            &UI::TextBlock::SetTextWrapping,
            textWrappingMetadata);

        PropertyMetadata paddingMetadata;
        paddingMetadata.DefaultValue = std::any(UI::Thickness{});
        paddingMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
        RegisterProperty<UI::TextBlock, UI::Thickness>(
            "Padding",
            &UI::TextBlock::GetPadding,
            [](UI::TextBlock* owner, const UI::Thickness& value) {
                if (owner) {
                    owner->SetPadding(value);
                }
            },
            paddingMetadata);
    }
}
