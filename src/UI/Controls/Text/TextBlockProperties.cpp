#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockForegroundMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Foreground));
        metadata.Converter = NeoWPF::UI::Properties::GetBrushConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockFontSizeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(16);
        metadata.Converter = NeoWPF::UI::Properties::GetIntConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockFontFamilyMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring(L"Segoe UI"));
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextHorizontalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextHorizontalAlignment::Left);
        metadata.Converter = NeoWPF::UI::Properties::GetTextHorizontalAlignmentConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextVerticalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextVerticalAlignment::Top);
        metadata.Converter = NeoWPF::UI::Properties::GetTextVerticalAlignmentConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockTextWrappingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextWrapping::NoWrap);
        metadata.Converter = NeoWPF::UI::Properties::GetTextWrappingConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockPaddingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Thickness{});
        metadata.Converter = NeoWPF::UI::Properties::GetThicknessConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockFontWeightMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(400);
        metadata.Converter = NeoWPF::UI::Properties::GetFontWeightConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBlockFontStyleMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::FontStyle::Normal);
        metadata.Converter = NeoWPF::UI::Properties::GetFontStyleConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterTextBlockProperties() {
        RegisterProperty<UI::TextBlock, std::wstring>("Text",
            &UI::TextBlock::GetText,
            &UI::TextBlock::SetText,
            MakeTextBlockTextMetadata());
        RegisterProperty<UI::TextBlock, std::shared_ptr<UI::Brush>>("Foreground",
            &UI::TextBlock::GetForeground,
            &UI::TextBlock::SetForeground,
            MakeTextBlockForegroundMetadata());
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
        RegisterProperty<UI::TextBlock, int>("FontWeight",
            &UI::TextBlock::GetFontWeight,
            &UI::TextBlock::SetFontWeight,
            MakeTextBlockFontWeightMetadata());
        RegisterProperty<UI::TextBlock, UI::FontStyle>("FontStyle",
            &UI::TextBlock::GetFontStyle,
            &UI::TextBlock::SetFontStyle,
            MakeTextBlockFontStyleMetadata());
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::TextBlock>(
            "TextBlock", "", "NeoWPF::UI::TextBlock", "NeoWPF/UI/Controls/Text/TextBlock.h");
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

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::ForegroundProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, std::shared_ptr<NeoWPF::UI::Brush>>(
        "Foreground",
        &NeoWPF::UI::TextBlock::GetForeground,
        &NeoWPF::UI::TextBlock::SetForeground,
        MakeTextBlockForegroundMetadata());
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

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::FontWeightProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, int>(
        "FontWeight",
        &NeoWPF::UI::TextBlock::GetFontWeight,
        &NeoWPF::UI::TextBlock::SetFontWeight,
        MakeTextBlockFontWeightMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBlock::FontStyleProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBlock, NeoWPF::UI::FontStyle>(
        "FontStyle",
        &NeoWPF::UI::TextBlock::GetFontStyle,
        &NeoWPF::UI::TextBlock::SetFontStyle,
        MakeTextBlockFontStyleMetadata());
    return *property;
}
