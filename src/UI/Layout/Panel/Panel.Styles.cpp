#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class PanelStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        element->SetStylePropertyValue("BackgroundColor", uint32_t(0xFF1E1E1Eu));
    }
};

}

namespace NeoWPF::UI {

void RegisterPanelStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(Panel), std::make_shared<Styling::PanelStyle>());
}

}
