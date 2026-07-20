#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyScrollBarThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("TrackColor", element->GetResourceValue("ScrollBar.TrackColor", uint32_t(0xFF2A2A2Au)));
    element->SetStylePropertyValue("ThumbColor", element->GetResourceValue("ScrollBar.ThumbColor", uint32_t(0xFF5A5A5Au)));
    element->SetStylePropertyValue("ThumbSize", element->GetResourceValue("ScrollBar.ThumbSize", 24));
}

}
