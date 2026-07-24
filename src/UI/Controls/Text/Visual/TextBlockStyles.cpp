#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyTextBlockThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }
    element->SetStylePropertyValue("TextColor", uint32_t(0xFF000000u));
    element->SetStylePropertyValue("FontSize", element->GetResourceValue("TextBlock.FontSize", 12));
}

}
