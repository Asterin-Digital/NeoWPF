#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeButtonTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeButtonTextColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFF000000u));
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
        RegisterProperty<UI::Button, std::wstring>("Text", MakeButtonTextMetadata());
        RegisterProperty<UI::Button, NeoWPF::UI::Color>("TextColor", MakeButtonTextColorMetadata());
        RegisterProperty<UI::Button, int>("FontSize", MakeButtonFontSizeMetadata());
    }
}

DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::Button,
                             Text,
                             std::wstring,
                             &NeoWPF::UI::Button::GetText,
                             &NeoWPF::UI::Button::SetText,
                             MakeButtonTextMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::Button,
                             TextColor,
                             NeoWPF::UI::Color,
                             &NeoWPF::UI::Button::GetTextColor,
                             static_cast<void (NeoWPF::UI::Button::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Button::SetTextColor),
                             MakeButtonTextColorMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::Button,
                             FontSize,
                             int,
                             &NeoWPF::UI::Button::GetFontSize,
                             &NeoWPF::UI::Button::SetFontSize,
                             MakeButtonFontSizeMetadata)
