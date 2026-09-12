#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
    using ::NeoWPF::UI::Color;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeCheckboxTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeCheckboxTextColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::Color::FromArgb(0xFF000000u));
        metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeCheckboxBackgroundColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu));
        metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeCheckboxBorderColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::Color::FromArgb(0xFF202020u));
        metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeCheckboxPaddingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::Thickness(8));
        metadata.Converter = NeoWPF::UI::Properties::GetThicknessConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeCheckboxFontSizeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(14);
        metadata.Converter = NeoWPF::UI::Properties::GetIntConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeCheckboxIsCheckedMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(false);
        metadata.Converter = NeoWPF::UI::Properties::GetBoolConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::None;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeCheckboxFontWeightMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(400);
        metadata.Converter = NeoWPF::UI::Properties::GetFontWeightConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeCheckboxFontStyleMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::FontStyle::Normal);
        metadata.Converter = NeoWPF::UI::Properties::GetFontStyleConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterCheckboxProperties() {
        RegisterProperty<UI::Checkbox, std::wstring>("Text",
            &UI::Checkbox::GetText,
            &UI::Checkbox::SetText,
            MakeCheckboxTextMetadata());
        RegisterProperty<UI::Checkbox, ::NeoWPF::UI::Color>("Foreground",
            &UI::Checkbox::GetTextColor,
            static_cast<void (UI::Checkbox::*)(::NeoWPF::UI::Color)>(&UI::Checkbox::SetTextColor),
            MakeCheckboxTextColorMetadata());
        RegisterProperty<UI::Checkbox, ::NeoWPF::UI::Color>("Background",
            &UI::Checkbox::GetBackgroundColor,
            static_cast<void (UI::Checkbox::*)(::NeoWPF::UI::Color)>(&UI::Checkbox::SetBackgroundColor),
            MakeCheckboxBackgroundColorMetadata());
        RegisterProperty<UI::Checkbox, ::NeoWPF::UI::Color>("BorderBrush",
            &UI::Checkbox::GetBorderColor,
            static_cast<void (UI::Checkbox::*)(::NeoWPF::UI::Color)>(&UI::Checkbox::SetBorderColor),
            MakeCheckboxBorderColorMetadata());
        RegisterProperty<UI::Checkbox, UI::Thickness>("Padding",
            &UI::Checkbox::GetPadding,
            static_cast<void (UI::Checkbox::*)(UI::Thickness)>(&UI::Checkbox::SetPadding),
            MakeCheckboxPaddingMetadata());
        RegisterProperty<UI::Checkbox, int>("FontSize",
            &UI::Checkbox::GetFontSize,
            &UI::Checkbox::SetFontSize,
            MakeCheckboxFontSizeMetadata());
        RegisterProperty<UI::Checkbox, int>("FontWeight",
            &UI::Checkbox::GetFontWeight,
            &UI::Checkbox::SetFontWeight,
            MakeCheckboxFontWeightMetadata());
        RegisterProperty<UI::Checkbox, UI::FontStyle>("FontStyle",
            &UI::Checkbox::GetFontStyle,
            &UI::Checkbox::SetFontStyle,
            MakeCheckboxFontStyleMetadata());
        RegisterProperty<UI::Checkbox, bool>("IsChecked",
            &UI::Checkbox::IsChecked,
            &UI::Checkbox::SetChecked,
            MakeCheckboxIsCheckedMetadata());
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Checkbox>(
            "Checkbox", "", "NeoWPF::UI::Checkbox", "NeoWPF/UI/Controls/Checkbox/Checkbox.h");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Checkbox::TextProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Checkbox, std::wstring>(
        "Text",
        &NeoWPF::UI::Checkbox::GetText,
        &NeoWPF::UI::Checkbox::SetText,
        MakeCheckboxTextMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Checkbox::TextColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Checkbox, NeoWPF::UI::Color>(
        "Foreground",
        &NeoWPF::UI::Checkbox::GetTextColor,
        static_cast<void (NeoWPF::UI::Checkbox::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Checkbox::SetTextColor),
        MakeCheckboxTextColorMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Checkbox::BackgroundColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Checkbox, NeoWPF::UI::Color>(
        "Background",
        &NeoWPF::UI::Checkbox::GetBackgroundColor,
        static_cast<void (NeoWPF::UI::Checkbox::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Checkbox::SetBackgroundColor),
        MakeCheckboxBackgroundColorMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Checkbox::BorderColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Checkbox, NeoWPF::UI::Color>(
        "BorderBrush",
        &NeoWPF::UI::Checkbox::GetBorderColor,
        static_cast<void (NeoWPF::UI::Checkbox::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Checkbox::SetBorderColor),
        MakeCheckboxBorderColorMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Checkbox::PaddingProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Checkbox, NeoWPF::UI::Thickness>(
        "Padding",
        &NeoWPF::UI::Checkbox::GetPadding,
        static_cast<void (NeoWPF::UI::Checkbox::*)(NeoWPF::UI::Thickness)>(&NeoWPF::UI::Checkbox::SetPadding),
        MakeCheckboxPaddingMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Checkbox::FontSizeProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Checkbox, int>(
        "FontSize",
        &NeoWPF::UI::Checkbox::GetFontSize,
        &NeoWPF::UI::Checkbox::SetFontSize,
        MakeCheckboxFontSizeMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Checkbox::FontWeightProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Checkbox, int>(
        "FontWeight",
        &NeoWPF::UI::Checkbox::GetFontWeight,
        &NeoWPF::UI::Checkbox::SetFontWeight,
        MakeCheckboxFontWeightMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Checkbox::FontStyleProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Checkbox, NeoWPF::UI::FontStyle>(
        "FontStyle",
        &NeoWPF::UI::Checkbox::GetFontStyle,
        &NeoWPF::UI::Checkbox::SetFontStyle,
        MakeCheckboxFontStyleMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Checkbox::IsCheckedProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Checkbox, bool>(
        "IsChecked",
        &NeoWPF::UI::Checkbox::IsChecked,
        &NeoWPF::UI::Checkbox::SetChecked,
        MakeCheckboxIsCheckedMetadata());
    return *property;
}
