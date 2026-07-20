#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyButtonThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("TextColor", element->GetResourceValue("Button.TextColor", uint32_t(0xFFEDEDEDu)));
    element->SetStylePropertyValue("FontSize", element->GetResourceValue("Button.FontSize", 14));
    element->SetStylePropertyValue("Padding", element->GetResourceValue("Button.Padding", 10));
    element->SetStylePropertyValue("BackgroundColor", element->GetResourceValue("Button.BackgroundColor", uint32_t(0xFF3A3A3Au)));

    element->SetStyleProperty("VisualState.Normal.Background", 0xFF3A3A3Au);
    element->SetStyleProperty("VisualState.MouseOver.Background", 0xFF5A7A9Au);
    element->SetStyleProperty("VisualState.Pressed.Background", 0xFF1A1A1Au);
    element->SetStyleProperty("VisualState.Disabled.Background", 0xFF2A2A2Au);
}

}
