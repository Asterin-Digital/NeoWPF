#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>

namespace NeoWPF::UI::Properties {
    void RegisterCoreControlProperties() {
        RegisterProperty<UI::Button, std::wstring>(
            "Text",
            &UI::Button::GetText,
            &UI::Button::SetText);

        RegisterProperty<UI::Button, uint32_t>(
            "TextColor",
            &UI::Button::GetTextColor,
            &UI::Button::SetTextColor);

        RegisterProperty<UI::TextBlock, std::wstring>(
            "Text",
            &UI::TextBlock::GetText,
            &UI::TextBlock::SetText);

        RegisterProperty<UI::TextBlock, uint32_t>(
            "TextColor",
            &UI::TextBlock::GetTextColor,
            &UI::TextBlock::SetTextColor);

        RegisterProperty<UI::TextBox, std::wstring>(
            "Text",
            &UI::TextBox::GetText,
            &UI::TextBox::SetText);

        RegisterProperty<UI::TextBox, uint32_t>(
            "TextColor",
            &UI::TextBox::GetTextColor,
            &UI::TextBox::SetTextColor);
    }
}
