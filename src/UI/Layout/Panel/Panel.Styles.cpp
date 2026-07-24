#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyPanelThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("BackgroundColor", element->GetResourceValue("Panel.BackgroundColor", uint32_t(0x00000000u)));
}

}
