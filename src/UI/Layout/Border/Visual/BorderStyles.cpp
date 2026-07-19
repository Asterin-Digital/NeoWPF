#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class BorderStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        element->SetStylePropertyValue("Padding", element->GetResourceValue("Border.Padding", 0));
        element->SetStylePropertyValue("BorderThickness", element->GetResourceValue("Border.BorderThickness", 1));
        element->SetStylePropertyValue("BorderColor", element->GetResourceValue("Border.BorderColor", uint32_t(0xFF333333u)));
        element->SetStylePropertyValue("BackgroundColor", element->GetResourceValue("Border.BackgroundColor", uint32_t(0xFF1E1E1Eu)));
    }
};

}

namespace NeoWPF::UI {

void RegisterBorderStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(Border), std::make_shared<Styling::BorderStyle>());
}

}
