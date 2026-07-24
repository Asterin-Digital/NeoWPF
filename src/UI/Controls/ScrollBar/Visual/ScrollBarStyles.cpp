#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyScrollBarThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("TrackColor", element->GetResourceValue("ScrollBar.TrackColor", uint32_t(0xFFEAEAEAu)));
    element->SetStylePropertyValue("ThumbColor", element->GetResourceValue("ScrollBar.ThumbColor", uint32_t(0xFFCDCDCDu)));
    element->SetStylePropertyValue("ThumbSize", element->GetResourceValue("ScrollBar.ThumbSize", 24));
}

}
