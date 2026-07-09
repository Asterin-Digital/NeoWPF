#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterTextBoxProperties() {
        RegisterProperty<UI::TextBox, std::wstring>(
            "Text",
            &UI::TextBox::GetText,
            &UI::TextBox::SetText);

        RegisterProperty<UI::TextBox, uint32_t>(
            "TextColor",
            &UI::TextBox::GetTextColor,
            &UI::TextBox::SetTextColor);

        RegisterProperty<UI::TextBox, uint32_t>(
            "BackgroundColor",
            &UI::TextBox::GetBackgroundColor,
            &UI::TextBox::SetBackgroundColor);

        RegisterProperty<UI::TextBox, uint32_t>(
            "BorderColor",
            &UI::TextBox::GetBorderColor,
            &UI::TextBox::SetBorderColor);

        RegisterProperty<UI::TextBox, uint32_t>(
            "CaretColor",
            &UI::TextBox::GetCaretColor,
            &UI::TextBox::SetCaretColor);

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
