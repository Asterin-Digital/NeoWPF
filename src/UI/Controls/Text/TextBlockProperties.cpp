#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterTextBlockProperties() {
        RegisterProperty<UI::TextBlock, std::wstring>(
            "Text",
            &UI::TextBlock::GetText,
            &UI::TextBlock::SetText);

        RegisterProperty<UI::TextBlock, NeoWPF::UI::Color>(
            "TextColor",
            &UI::TextBlock::GetTextColor,
            static_cast<void (UI::TextBlock::*)(NeoWPF::UI::Color)>(&UI::TextBlock::SetTextColor));

        RegisterProperty<UI::TextBlock, int>(
            "FontSize",
            &UI::TextBlock::GetFontSize,
            &UI::TextBlock::SetFontSize);
    }
}
