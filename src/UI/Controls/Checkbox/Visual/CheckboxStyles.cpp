#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyCheckboxThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("TextColor", element->GetResourceValue("Checkbox.TextColor", NeoWPF::UI::Color::FromArgb(0xFFEDEDEDu)));
    element->SetStylePropertyValue("BoxColor", element->GetResourceValue("Checkbox.BoxColor", NeoWPF::UI::Color::FromArgb(0xFF3A3A3Au)));
    element->SetStylePropertyValue("BorderColor", element->GetResourceValue("Checkbox.BorderColor", NeoWPF::UI::Color::FromArgb(0xFF666666u)));
}

}
