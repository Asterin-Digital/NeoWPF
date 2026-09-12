#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
    using ::NeoWPF::UI::Color;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxPlaceholderTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender | NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxTextColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::Color::FromArgb(0xFF000000u));
        metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxCaretColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu));
        metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxFontSizeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(16);
        metadata.Converter = NeoWPF::UI::Properties::GetIntConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxFontFamilyMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxTextHorizontalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextHorizontalAlignment::Left);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxTextVerticalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextVerticalAlignment::Top);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxFontWeightMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(400);
        metadata.Converter = NeoWPF::UI::Properties::GetFontWeightConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxFontStyleMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::FontStyle::Normal);
        metadata.Converter = NeoWPF::UI::Properties::GetFontStyleConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterTextBoxProperties() {
        RegisterProperty<UI::TextBox, std::wstring>("Text",
            &UI::TextBox::GetText,
            static_cast<void (UI::TextBox::*)(const std::wstring&)>(&UI::TextBox::SetText),
            MakeTextBoxTextMetadata());
        RegisterProperty<UI::TextBox, std::wstring>("PlaceholderText",
            &UI::TextBox::GetPlaceholderText,
            &UI::TextBox::SetPlaceholderText,
            MakeTextBoxPlaceholderTextMetadata());
        RegisterProperty<UI::TextBox, ::NeoWPF::UI::Color>("Foreground",
            &UI::TextBox::GetTextColor,
            static_cast<void (UI::TextBox::*)(::NeoWPF::UI::Color)>(&UI::TextBox::SetTextColor),
            MakeTextBoxTextColorMetadata());
        RegisterProperty<UI::TextBox, ::NeoWPF::UI::Color>("CaretBrush",
            &UI::TextBox::GetCaretColor,
            static_cast<void (UI::TextBox::*)(::NeoWPF::UI::Color)>(&UI::TextBox::SetCaretColor),
            MakeTextBoxCaretColorMetadata());
        RegisterProperty<UI::TextBox, int>("FontSize",
            &UI::TextBox::GetFontSize,
            &UI::TextBox::SetFontSize,
            MakeTextBoxFontSizeMetadata());
        RegisterProperty<UI::TextBox, std::wstring>("FontFamily",
            &UI::TextBox::GetFontFamily,
            &UI::TextBox::SetFontFamily,
            MakeTextBoxFontFamilyMetadata());
        RegisterProperty<UI::TextBox, TextHorizontalAlignment>("TextHorizontalAlignment",
            &UI::TextBox::GetTextHorizontalAlignment,
            static_cast<void (UI::TextBox::*)(TextHorizontalAlignment)>(&UI::TextBox::SetTextHorizontalAlignment),
            MakeTextBoxTextHorizontalAlignmentMetadata());
        RegisterProperty<UI::TextBox, TextVerticalAlignment>("TextVerticalAlignment",
            &UI::TextBox::GetTextVerticalAlignment,
            static_cast<void (UI::TextBox::*)(TextVerticalAlignment)>(&UI::TextBox::SetTextVerticalAlignment),
            MakeTextBoxTextVerticalAlignmentMetadata());
        RegisterProperty<UI::TextBox, int>("FontWeight",
            &UI::TextBox::GetFontWeight,
            &UI::TextBox::SetFontWeight,
            MakeTextBoxFontWeightMetadata());
        RegisterProperty<UI::TextBox, UI::FontStyle>("FontStyle",
            &UI::TextBox::GetFontStyle,
            &UI::TextBox::SetFontStyle,
            MakeTextBoxFontStyleMetadata());
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::TextBox>(
            "TextBox", "", "NeoWPF::UI::TextBox", "NeoWPF/UI/Controls/TextBox/TextBox.h");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::TextProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, std::wstring>(
        "Text",
        &NeoWPF::UI::TextBox::GetText,
        static_cast<void (NeoWPF::UI::TextBox::*)(const std::wstring&)>(&NeoWPF::UI::TextBox::SetText),
        MakeTextBoxTextMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::PlaceholderTextProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, std::wstring>(
        "PlaceholderText",
        &NeoWPF::UI::TextBox::GetPlaceholderText,
        &NeoWPF::UI::TextBox::SetPlaceholderText,
        MakeTextBoxPlaceholderTextMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::TextColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, NeoWPF::UI::Color>(
        "Foreground",
        &NeoWPF::UI::TextBox::GetTextColor,
        static_cast<void (NeoWPF::UI::TextBox::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::TextBox::SetTextColor),
        MakeTextBoxTextColorMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::CaretColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, NeoWPF::UI::Color>(
        "CaretBrush",
        &NeoWPF::UI::TextBox::GetCaretColor,
        static_cast<void (NeoWPF::UI::TextBox::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::TextBox::SetCaretColor),
        MakeTextBoxCaretColorMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::FontSizeProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, int>(
        "FontSize",
        &NeoWPF::UI::TextBox::GetFontSize,
        &NeoWPF::UI::TextBox::SetFontSize,
        MakeTextBoxFontSizeMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::FontFamilyProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, std::wstring>(
        "FontFamily",
        &NeoWPF::UI::TextBox::GetFontFamily,
        &NeoWPF::UI::TextBox::SetFontFamily,
        MakeTextBoxFontFamilyMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::TextHorizontalAlignmentProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, TextHorizontalAlignment>(
        "TextHorizontalAlignment",
        &NeoWPF::UI::TextBox::GetTextHorizontalAlignment,
        static_cast<void (NeoWPF::UI::TextBox::*)(TextHorizontalAlignment)>(&NeoWPF::UI::TextBox::SetTextHorizontalAlignment),
        MakeTextBoxTextHorizontalAlignmentMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::TextVerticalAlignmentProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, TextVerticalAlignment>(
        "TextVerticalAlignment",
        &NeoWPF::UI::TextBox::GetTextVerticalAlignment,
        static_cast<void (NeoWPF::UI::TextBox::*)(TextVerticalAlignment)>(&NeoWPF::UI::TextBox::SetTextVerticalAlignment),
        MakeTextBoxTextVerticalAlignmentMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::FontWeightProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, int>(
        "FontWeight",
        &NeoWPF::UI::TextBox::GetFontWeight,
        &NeoWPF::UI::TextBox::SetFontWeight,
        MakeTextBoxFontWeightMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::FontStyleProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::TextBox, NeoWPF::UI::FontStyle>(
        "FontStyle",
        &NeoWPF::UI::TextBox::GetFontStyle,
        &NeoWPF::UI::TextBox::SetFontStyle,
        MakeTextBoxFontStyleMetadata());
    return *property;
}
