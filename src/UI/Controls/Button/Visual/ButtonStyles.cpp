#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyButtonThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("TextColor", element->GetResourceValue("Button.TextColor", uint32_t(0xFF000000u)));
    element->SetStylePropertyValue("FontSize", element->GetResourceValue("Button.FontSize", 14));
    element->SetStylePropertyValue("Padding", element->GetResourceValue("Button.Padding", 10));
    element->SetStylePropertyValue("BackgroundColor", element->GetResourceValue("Button.BackgroundColor", NeoWPF::UI::Color::FromArgb(0xFFEEEEEEu)));
    element->SetStylePropertyValue("BorderColor", element->GetResourceValue("Button.BorderColor", uint32_t(0xFF707070u)));

    element->SetStyleProperty("VisualState.Normal.Background", 0xFFEEEEEEu);
    element->SetStyleProperty("VisualState.MouseOver.Background", 0xFFBEE6FDu);
    element->SetStyleProperty("VisualState.Pressed.Background", 0xFFC4E5F6u);
    element->SetStyleProperty("VisualState.Disabled.Background", 0xFFF4F4F4u);
}

}
