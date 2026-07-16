#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterButtonProperties() {
        RegisterProperty<UI::Button, std::wstring>(
            "Text",
            &UI::Button::GetText,
            &UI::Button::SetText);

        RegisterProperty<UI::Button, NeoWPF::UI::Color>(
            "TextColor",
            &UI::Button::GetTextColor,
            static_cast<void (UI::Button::*)(NeoWPF::UI::Color)>(&UI::Button::SetTextColor));

        RegisterProperty<UI::Button, int>(
            "FontSize",
            &UI::Button::GetFontSize,
            &UI::Button::SetFontSize);

        RegisterProperty<UI::Button, int>(
            "Padding",
            &UI::Button::GetPadding,
            &UI::Button::SetPadding);

        RegisterProperty<UI::Button, NeoWPF::UI::Color>(
            "BackgroundColor",
            &UI::Button::GetBackgroundColor,
            static_cast<void (UI::Button::*)(NeoWPF::UI::Color)>(&UI::Button::SetBackgroundColor));

        RegisterProperty<UI::Button, NeoWPF::UI::Color>(
            "BorderColor",
            &UI::Button::GetBorderColor,
            static_cast<void (UI::Button::*)(NeoWPF::UI::Color)>(&UI::Button::SetBorderColor));
    }
}
