#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterTextBoxProperties() {
        RegisterProperty<UI::TextBox, std::wstring>(
            "Text",
            &UI::TextBox::GetText,
            &UI::TextBox::SetText);

        RegisterProperty<UI::TextBox, NeoWPF::UI::Color>(
            "TextColor",
            &UI::TextBox::GetTextColor,
            static_cast<void (UI::TextBox::*)(NeoWPF::UI::Color)>(&UI::TextBox::SetTextColor));

        RegisterProperty<UI::TextBox, NeoWPF::UI::Color>(
            "BackgroundColor",
            &UI::TextBox::GetBackgroundColor,
            static_cast<void (UI::TextBox::*)(NeoWPF::UI::Color)>(&UI::TextBox::SetBackgroundColor));

        RegisterProperty<UI::TextBox, NeoWPF::UI::Color>(
            "BorderColor",
            &UI::TextBox::GetBorderColor,
            static_cast<void (UI::TextBox::*)(NeoWPF::UI::Color)>(&UI::TextBox::SetBorderColor));

        RegisterProperty<UI::TextBox, NeoWPF::UI::Color>(
            "CaretColor",
            &UI::TextBox::GetCaretColor,
            static_cast<void (UI::TextBox::*)(NeoWPF::UI::Color)>(&UI::TextBox::SetCaretColor));

        RegisterProperty<UI::TextBox, int>(
            "FontSize",
            &UI::TextBox::GetFontSize,
            &UI::TextBox::SetFontSize);

        RegisterProperty<UI::TextBox, int>(
            "Padding",
            &UI::TextBox::GetPadding,
            &UI::TextBox::SetPadding);
    }
}
