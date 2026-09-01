#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyBorderThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("Padding", element->GetResourceValue("Border.Padding", 0));
    element->SetStylePropertyValue("BorderThickness", element->GetResourceValue("Border.BorderThickness", NeoWPF::UI::Thickness(1)));
    element->SetStylePropertyValue("BorderBrush", element->GetResourceValue("Border.BorderColor", uint32_t(0xFF707070u)));
    element->SetStylePropertyValue("Background", element->GetResourceValue("Border.BackgroundColor", uint32_t(0xFFFFFFFFu)));
}

}
