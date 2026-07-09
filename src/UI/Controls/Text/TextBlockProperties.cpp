#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterTextBlockProperties() {
        RegisterProperty<UI::TextBlock, std::wstring>(
            "Text",
            &UI::TextBlock::GetText,
            &UI::TextBlock::SetText);

        RegisterProperty<UI::TextBlock, uint32_t>(
            "TextColor",
            &UI::TextBlock::GetTextColor,
            &UI::TextBlock::SetTextColor);

        RegisterProperty<UI::TextBlock, int>(
            "FontSize",
            &UI::TextBlock::GetFontSize,
            &UI::TextBlock::SetFontSize);
    }
}
