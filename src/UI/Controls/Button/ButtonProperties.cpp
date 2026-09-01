#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
    using ::NeoWPF::UI::Color;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeButtonTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonTextColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(::NeoWPF::UI::Color::FromArgb(0xFF000000u));
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonFontSizeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(16);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterButtonProperties() {
        RegisterProperty<UI::Button, std::wstring>("Text",
            &UI::Button::GetText,
            &UI::Button::SetText,
            MakeButtonTextMetadata());
        RegisterProperty<UI::Button, ::NeoWPF::UI::Color>("Foreground",
            &UI::Button::GetTextColor,
            static_cast<void (UI::Button::*)(::NeoWPF::UI::Color)>(&UI::Button::SetTextColor),
            MakeButtonTextColorMetadata());
        RegisterProperty<UI::Button, int>("FontSize",
            &UI::Button::GetFontSize,
            &UI::Button::SetFontSize,
            MakeButtonFontSizeMetadata());
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Button>("Button");
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

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Button::TextColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Button, NeoWPF::UI::Color>(
        "Foreground",
        &NeoWPF::UI::Button::GetTextColor,
        static_cast<void (NeoWPF::UI::Button::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Button::SetTextColor),
        MakeButtonTextColorMetadata());
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
