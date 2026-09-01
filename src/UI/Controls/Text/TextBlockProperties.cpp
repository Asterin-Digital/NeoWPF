#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
    using ::NeoWPF::UI::Color;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::Color::FromArgb(0xFF000000u));
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
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextHorizontalAlignment::Left);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextVerticalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextVerticalAlignment::Top);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextWrappingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextWrapping::NoWrap);
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
        RegisterProperty<UI::TextBlock, std::wstring>("Text",
            &UI::TextBlock::GetText,
            &UI::TextBlock::SetText,
            MakeTextBlockTextMetadata());
        RegisterProperty<UI::TextBlock, ::NeoWPF::UI::Color>("Foreground",
            &UI::TextBlock::GetTextColor,
            static_cast<void (UI::TextBlock::*)(::NeoWPF::UI::Color)>(&UI::TextBlock::SetTextColor),
            MakeTextBlockTextColorMetadata());
        RegisterProperty<UI::TextBlock, int>("FontSize",
            &UI::TextBlock::GetFontSize,
            &UI::TextBlock::SetFontSize,
            MakeTextBlockFontSizeMetadata());
        RegisterProperty<UI::TextBlock, std::wstring>("FontFamily",
            &UI::TextBlock::GetFontFamily,
            &UI::TextBlock::SetFontFamily,
            MakeTextBlockFontFamilyMetadata());
        RegisterProperty<UI::TextBlock, TextHorizontalAlignment>("TextHorizontalAlignment",
            &UI::TextBlock::GetTextHorizontalAlignment,
            static_cast<void (UI::TextBlock::*)(TextHorizontalAlignment)>(&UI::TextBlock::SetTextHorizontalAlignment),
            MakeTextBlockTextHorizontalAlignmentMetadata());
        RegisterProperty<UI::TextBlock, TextVerticalAlignment>("TextVerticalAlignment",
            &UI::TextBlock::GetTextVerticalAlignment,
            static_cast<void (UI::TextBlock::*)(TextVerticalAlignment)>(&UI::TextBlock::SetTextVerticalAlignment),
            MakeTextBlockTextVerticalAlignmentMetadata());
        RegisterProperty<UI::TextBlock, TextWrapping>("TextWrapping",
            &UI::TextBlock::GetTextWrapping,
            &UI::TextBlock::SetTextWrapping,
            MakeTextBlockTextWrappingMetadata());
        RegisterProperty<UI::TextBlock, UI::Thickness>("Padding",
            &UI::TextBlock::GetPadding,
            static_cast<void (UI::TextBlock::*)(UI::Thickness)>(&UI::TextBlock::SetPadding),
            MakeTextBlockPaddingMetadata());
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::TextBlock>("TextBlock");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::TextProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, std::wstring>(
        "Text",
        &NeoWPF::UI::TextBlock::GetText,
        &NeoWPF::UI::TextBlock::SetText,
        MakeTextBlockTextMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::TextColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, NeoWPF::UI::Color>(
        "Foreground",
        &NeoWPF::UI::TextBlock::GetTextColor,
        static_cast<void (NeoWPF::UI::TextBlock::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::TextBlock::SetTextColor),
        MakeTextBlockTextColorMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::FontSizeProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, int>(
        "FontSize",
        &NeoWPF::UI::TextBlock::GetFontSize,
        &NeoWPF::UI::TextBlock::SetFontSize,
        MakeTextBlockFontSizeMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::FontFamilyProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, std::wstring>(
        "FontFamily",
        &NeoWPF::UI::TextBlock::GetFontFamily,
        &NeoWPF::UI::TextBlock::SetFontFamily,
        MakeTextBlockFontFamilyMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::TextHorizontalAlignmentProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, TextHorizontalAlignment>(
        "TextHorizontalAlignment",
        &NeoWPF::UI::TextBlock::GetTextHorizontalAlignment,
        static_cast<void (NeoWPF::UI::TextBlock::*)(TextHorizontalAlignment)>(&NeoWPF::UI::TextBlock::SetTextHorizontalAlignment),
        MakeTextBlockTextHorizontalAlignmentMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::TextVerticalAlignmentProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, TextVerticalAlignment>(
        "TextVerticalAlignment",
        &NeoWPF::UI::TextBlock::GetTextVerticalAlignment,
        static_cast<void (NeoWPF::UI::TextBlock::*)(TextVerticalAlignment)>(&NeoWPF::UI::TextBlock::SetTextVerticalAlignment),
        MakeTextBlockTextVerticalAlignmentMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::TextWrappingProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, TextWrapping>(
        "TextWrapping",
        &NeoWPF::UI::TextBlock::GetTextWrapping,
        &NeoWPF::UI::TextBlock::SetTextWrapping,
        MakeTextBlockTextWrappingMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::PaddingProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, NeoWPF::UI::Thickness>(
        "Padding",
        &NeoWPF::UI::TextBlock::GetPadding,
        static_cast<void (NeoWPF::UI::TextBlock::*)(NeoWPF::UI::Thickness)>(&NeoWPF::UI::TextBlock::SetPadding),
        MakeTextBlockPaddingMetadata());
    return *property;
}
