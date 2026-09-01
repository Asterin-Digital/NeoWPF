#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::None;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxPlaceholderTextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender | NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxTextColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFFEDEDEDu));
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxCaretColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu));
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxFontSizeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(16);
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
        metadata.DefaultValue = std::any(NeoWPF::UI::TextHorizontalAlignment::Left);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeTextBoxTextVerticalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::TextVerticalAlignment::Top);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterTextBoxProperties() {
        RegisterProperty<UI::TextBox, std::wstring>("Text", MakeTextBoxTextMetadata());
        RegisterProperty<UI::TextBox, std::wstring>("PlaceholderText", MakeTextBoxPlaceholderTextMetadata());
        RegisterProperty<UI::TextBox, NeoWPF::UI::Color>("TextColor", MakeTextBoxTextColorMetadata());
        RegisterProperty<UI::TextBox, NeoWPF::UI::Color>("CaretColor", MakeTextBoxCaretColorMetadata());
        RegisterProperty<UI::TextBox, int>("FontSize", MakeTextBoxFontSizeMetadata());
        RegisterProperty<UI::TextBox, std::wstring>("FontFamily", MakeTextBoxFontFamilyMetadata());
        RegisterProperty<UI::TextBox, TextHorizontalAlignment>("TextHorizontalAlignment", MakeTextBoxTextHorizontalAlignmentMetadata());
        RegisterProperty<UI::TextBox, TextVerticalAlignment>("TextVerticalAlignment", MakeTextBoxTextVerticalAlignmentMetadata());
    }
}

DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBox,
                             Text,
                             std::wstring,
                             &NeoWPF::UI::TextBox::GetText,
                             static_cast<void (NeoWPF::UI::TextBox::*)(const std::wstring&)>(&NeoWPF::UI::TextBox::SetText),
                             MakeTextBoxTextMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBox,
                             PlaceholderText,
                             std::wstring,
                             &NeoWPF::UI::TextBox::GetPlaceholderText,
                             &NeoWPF::UI::TextBox::SetPlaceholderText,
                             MakeTextBoxPlaceholderTextMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBox,
                             TextColor,
                             NeoWPF::UI::Color,
                             &NeoWPF::UI::TextBox::GetTextColor,
                             static_cast<void (NeoWPF::UI::TextBox::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::TextBox::SetTextColor),
                             MakeTextBoxTextColorMetadata) 
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBox,
                             CaretColor,
                             NeoWPF::UI::Color,
                             &NeoWPF::UI::TextBox::GetCaretColor,
                             static_cast<void (NeoWPF::UI::TextBox::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::TextBox::SetCaretColor),
                             MakeTextBoxCaretColorMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBox,
                             FontSize,
                             int,
                             &NeoWPF::UI::TextBox::GetFontSize,
                             &NeoWPF::UI::TextBox::SetFontSize,
                             MakeTextBoxFontSizeMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBox,
                             FontFamily,
                             std::wstring,
                             &NeoWPF::UI::TextBox::GetFontFamily,
                             &NeoWPF::UI::TextBox::SetFontFamily,
                             MakeTextBoxFontFamilyMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBox,
                             TextHorizontalAlignment,
                             TextHorizontalAlignment,
                             &NeoWPF::UI::TextBox::GetTextHorizontalAlignment,
                             static_cast<void (NeoWPF::UI::TextBox::*)(TextHorizontalAlignment)>(&NeoWPF::UI::TextBox::SetTextHorizontalAlignment),
                             MakeTextBoxTextHorizontalAlignmentMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::TextBox,
                             TextVerticalAlignment,
                             TextVerticalAlignment,
                             &NeoWPF::UI::TextBox::GetTextVerticalAlignment,
                             static_cast<void (NeoWPF::UI::TextBox::*)(TextVerticalAlignment)>(&NeoWPF::UI::TextBox::SetTextVerticalAlignment),
                             MakeTextBoxTextVerticalAlignmentMetadata)
