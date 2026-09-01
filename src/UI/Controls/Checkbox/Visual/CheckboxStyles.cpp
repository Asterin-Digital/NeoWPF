#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyCheckboxThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("Foreground", element->GetResourceValue("Checkbox.TextColor", NeoWPF::UI::Color::FromArgb(0xFF000000u)));
    element->SetStylePropertyValue("Background", element->GetResourceValue("Checkbox.BoxColor", NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu)));
    element->SetStylePropertyValue("BorderBrush", element->GetResourceValue("Checkbox.BorderColor", NeoWPF::UI::Color::FromArgb(0xFF202020u)));
}

}
