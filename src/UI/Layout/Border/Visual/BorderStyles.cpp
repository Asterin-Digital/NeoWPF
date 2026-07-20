#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyBorderThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("Padding", element->GetResourceValue("Border.Padding", 0));
    element->SetStylePropertyValue("BorderThickness", element->GetResourceValue("Border.BorderThickness", 1));
    element->SetStylePropertyValue("BorderColor", element->GetResourceValue("Border.BorderColor", uint32_t(0xFF333333u)));
    element->SetStylePropertyValue("BackgroundColor", element->GetResourceValue("Border.BackgroundColor", uint32_t(0xFF1E1E1Eu)));
}

}
