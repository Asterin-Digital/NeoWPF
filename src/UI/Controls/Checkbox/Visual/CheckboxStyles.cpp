#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyCheckboxThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("TextColor", element->GetResourceValue("Checkbox.TextColor", NeoWPF::UI::Color::FromArgb(0xFF000000u)));
    element->SetStylePropertyValue("BoxColor", element->GetResourceValue("Checkbox.BoxColor", NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu)));
    element->SetStylePropertyValue("BorderColor", element->GetResourceValue("Checkbox.BorderColor", NeoWPF::UI::Color::FromArgb(0xFF202020u)));
}

}
