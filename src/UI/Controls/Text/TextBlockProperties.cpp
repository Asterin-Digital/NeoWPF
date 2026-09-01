#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFF000000u));
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockFontSizeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(16);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockFontFamilyMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextHorizontalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::TextHorizontalAlignment::Left);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextVerticalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::TextVerticalAlignment::Top);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextWrappingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::TextWrapping::NoWrap);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockPaddingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Thickness{});
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterTextBlockProperties() {
        RegisterProperty<UI::TextBlock, std::wstring>("Text", MakeTextBlockTextMetadata());
        RegisterProperty<UI::TextBlock, NeoWPF::UI::Color>("TextColor", MakeTextBlockTextColorMetadata());
        RegisterProperty<UI::TextBlock, int>("FontSize", MakeTextBlockFontSizeMetadata());
        RegisterProperty<UI::TextBlock, std::wstring>("FontFamily", MakeTextBlockFontFamilyMetadata());
        RegisterProperty<UI::TextBlock, TextHorizontalAlignment>("TextHorizontalAlignment", MakeTextBlockTextHorizontalAlignmentMetadata());
        RegisterProperty<UI::TextBlock, TextVerticalAlignment>("TextVerticalAlignment", MakeTextBlockTextVerticalAlignmentMetadata());
        RegisterProperty<UI::TextBlock, TextWrapping>("TextWrapping", MakeTextBlockTextWrappingMetadata());
        RegisterProperty<UI::TextBlock, UI::Thickness>("Padding", MakeTextBlockPaddingMetadata());
    }
}

DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBlock,
                             Text,
                             std::wstring,
                             &NeoWPF::UI::TextBlock::GetText,
                             &NeoWPF::UI::TextBlock::SetText,
                             MakeTextBlockTextMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBlock,
                             TextColor,
                             NeoWPF::UI::Color,
                             &NeoWPF::UI::TextBlock::GetTextColor,
                             static_cast<void (NeoWPF::UI::TextBlock::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::TextBlock::SetTextColor),
                             MakeTextBlockTextColorMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBlock,
                             FontSize,
                             int,
                             &NeoWPF::UI::TextBlock::GetFontSize,
                             &NeoWPF::UI::TextBlock::SetFontSize,
                             MakeTextBlockFontSizeMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBlock,
                             FontFamily,
                             std::wstring,
                             &NeoWPF::UI::TextBlock::GetFontFamily,
                             &NeoWPF::UI::TextBlock::SetFontFamily,
                             MakeTextBlockFontFamilyMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBlock,
                             TextHorizontalAlignment,
                             TextHorizontalAlignment,
                             &NeoWPF::UI::TextBlock::GetTextHorizontalAlignment,
                             static_cast<void (NeoWPF::UI::TextBlock::*)(TextHorizontalAlignment)>(&NeoWPF::UI::TextBlock::SetTextHorizontalAlignment),
                             MakeTextBlockTextHorizontalAlignmentMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBlock,
                             TextVerticalAlignment,
                             TextVerticalAlignment,
                             &NeoWPF::UI::TextBlock::GetTextVerticalAlignment,
                             static_cast<void (NeoWPF::UI::TextBlock::*)(TextVerticalAlignment)>(&NeoWPF::UI::TextBlock::SetTextVerticalAlignment),
                             MakeTextBlockTextVerticalAlignmentMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBlock,
                             TextWrapping,
                             TextWrapping,
                             &NeoWPF::UI::TextBlock::GetTextWrapping,
                             &NeoWPF::UI::TextBlock::SetTextWrapping,
                             MakeTextBlockTextWrappingMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBlock,
                             Padding,
                             UI::Thickness,
                             &NeoWPF::UI::TextBlock::GetPadding,
                             static_cast<void (NeoWPF::UI::TextBlock::*)(UI::Thickness)>(&NeoWPF::UI::TextBlock::SetPadding),
                             MakeTextBlockPaddingMetadata)
