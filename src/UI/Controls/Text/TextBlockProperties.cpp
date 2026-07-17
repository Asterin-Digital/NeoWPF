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
    }
}
