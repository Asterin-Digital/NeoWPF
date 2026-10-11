#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeButtonTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonForegroundMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::shared_ptr<NeoWPF::UI::Brush>{});
        metadata.Converter = NeoWPF::UI::Properties::GetBrushConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonFontSizeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(16);
        metadata.Converter = NeoWPF::UI::Properties::GetIntConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonFontFamilyMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonFontWeightMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(400);
        metadata.Converter = NeoWPF::UI::Properties::GetFontWeightConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonFontStyleMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::FontStyle::Normal);
        metadata.Converter = NeoWPF::UI::Properties::GetFontStyleConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonTextHorizontalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextHorizontalAlignment::Center);
        metadata.Converter = NeoWPF::UI::Properties::GetTextHorizontalAlignmentConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonTextVerticalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::TextVerticalAlignment::Center);
        metadata.Converter = NeoWPF::UI::Properties::GetTextVerticalAlignmentConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterButtonProperties() {
        RegisterProperty<UI::Button, std::wstring>("Text",
            &UI::Button::GetText,
            &UI::Button::SetText,
            MakeButtonTextMetadata());
        RegisterProperty<UI::Button, std::shared_ptr<UI::Brush>>("Foreground",
            &UI::Button::GetForeground,
            &UI::Button::SetForeground,
            MakeButtonForegroundMetadata());
        RegisterProperty<UI::Button, int>("FontSize",
            &UI::Button::GetFontSize,
            &UI::Button::SetFontSize,
            MakeButtonFontSizeMetadata());
        RegisterProperty<UI::Button, std::wstring>("FontFamily",
            &UI::Button::GetFontFamily,
            &UI::Button::SetFontFamily,
            MakeButtonFontFamilyMetadata());
        RegisterProperty<UI::Button, int>("FontWeight",
            &UI::Button::GetFontWeight,
            &UI::Button::SetFontWeight,
            MakeButtonFontWeightMetadata());
        RegisterProperty<UI::Button, UI::FontStyle>("FontStyle",
            &UI::Button::GetFontStyle,
            &UI::Button::SetFontStyle,
            MakeButtonFontStyleMetadata());
        RegisterProperty<UI::Button, TextHorizontalAlignment>("TextHorizontalAlignment",
            &UI::Button::GetTextHorizontalAlignment,
            static_cast<void (UI::Button::*)(TextHorizontalAlignment)>(&UI::Button::SetTextHorizontalAlignment),
            MakeButtonTextHorizontalAlignmentMetadata());
        RegisterProperty<UI::Button, TextVerticalAlignment>("TextVerticalAlignment",
            &UI::Button::GetTextVerticalAlignment,
            static_cast<void (UI::Button::*)(TextVerticalAlignment)>(&UI::Button::SetTextVerticalAlignment),
            MakeButtonTextVerticalAlignmentMetadata());
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Button>(
            "Button", "", "NeoWPF::UI::Button", "NeoWPF/UI/Controls/Button/Button.h");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Button::TextProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Button, std::wstring>(
        "Text",
        &NeoWPF::UI::Button::GetText,
        &NeoWPF::UI::Button::SetText,
        MakeButtonTextMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Button::ForegroundProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Button, std::shared_ptr<NeoWPF::UI::Brush>>(
        "Foreground",
        &NeoWPF::UI::Button::GetForeground,
        &NeoWPF::UI::Button::SetForeground,
        MakeButtonForegroundMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Button::FontSizeProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Button, int>(
        "FontSize",
        &NeoWPF::UI::Button::GetFontSize,
        &NeoWPF::UI::Button::SetFontSize,
        MakeButtonFontSizeMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Button::FontFamilyProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Button, std::wstring>(
        "FontFamily",
        &NeoWPF::UI::Button::GetFontFamily,
        &NeoWPF::UI::Button::SetFontFamily,
        MakeButtonFontFamilyMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Button::FontWeightProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Button, int>(
        "FontWeight",
        &NeoWPF::UI::Button::GetFontWeight,
        &NeoWPF::UI::Button::SetFontWeight,
        MakeButtonFontWeightMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Button::FontStyleProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Button, NeoWPF::UI::FontStyle>(
        "FontStyle",
        &NeoWPF::UI::Button::GetFontStyle,
        &NeoWPF::UI::Button::SetFontStyle,
        MakeButtonFontStyleMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Button::TextHorizontalAlignmentProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Button, TextHorizontalAlignment>(
        "TextHorizontalAlignment",
        &NeoWPF::UI::Button::GetTextHorizontalAlignment,
        static_cast<void (NeoWPF::UI::Button::*)(TextHorizontalAlignment)>(&NeoWPF::UI::Button::SetTextHorizontalAlignment),
        MakeButtonTextHorizontalAlignmentMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Button::TextVerticalAlignmentProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Button, TextVerticalAlignment>(
        "TextVerticalAlignment",
        &NeoWPF::UI::Button::GetTextVerticalAlignment,
        static_cast<void (NeoWPF::UI::Button::*)(TextVerticalAlignment)>(&NeoWPF::UI::Button::SetTextVerticalAlignment),
        MakeButtonTextVerticalAlignmentMetadata());
    return *property;
}
