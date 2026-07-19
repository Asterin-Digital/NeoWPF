#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class TextBlockStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        element->SetStylePropertyValue("TextColor", element->GetResourceValue("TextBlock.TextColor", uint32_t(0xFFEDEDEDu)));
        element->SetStylePropertyValue("FontSize", element->GetResourceValue("TextBlock.FontSize", 12));
    }
};

}

namespace NeoWPF::UI {

void RegisterTextBlockStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(TextBlock), std::make_shared<Styling::TextBlockStyle>());
}

}
