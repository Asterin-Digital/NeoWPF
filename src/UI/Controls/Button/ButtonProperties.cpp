#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterButtonProperties() {
        RegisterProperty<UI::Button, std::wstring>(
            "Text",
            &UI::Button::GetText,
            &UI::Button::SetText);

        RegisterProperty<UI::Button, uint32_t>(
            "TextColor",
            &UI::Button::GetTextColor,
            &UI::Button::SetTextColor);

        RegisterProperty<UI::Button, int>(
            "FontSize",
            &UI::Button::GetFontSize,
            &UI::Button::SetFontSize);

        RegisterProperty<UI::Button, int>(
            "Padding",
            &UI::Button::GetPadding,
            &UI::Button::SetPadding);

        RegisterProperty<UI::Button, uint32_t>(
            "BackgroundColor",
            &UI::Button::GetBackgroundColor,
            &UI::Button::SetBackgroundColor);
    }
}
