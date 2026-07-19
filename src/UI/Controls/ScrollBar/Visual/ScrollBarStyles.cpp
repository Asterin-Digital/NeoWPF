#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class ScrollBarStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        element->SetStylePropertyValue("TrackColor", element->GetResourceValue("ScrollBar.TrackColor", uint32_t(0xFF2A2A2Au)));
        element->SetStylePropertyValue("ThumbColor", element->GetResourceValue("ScrollBar.ThumbColor", uint32_t(0xFF5A5A5Au)));
        element->SetStylePropertyValue("ThumbSize", element->GetResourceValue("ScrollBar.ThumbSize", 24));
    }
};

}

namespace NeoWPF::UI {

void RegisterScrollBarStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(ScrollBar), std::make_shared<Styling::ScrollBarStyle>());
}

}
