#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Styling/StyleManager.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class PanelStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        if (auto panel = dynamic_cast<UI::Panel*>(element)) {
            panel->SetBackgroundColor(0xFF1E1E1Eu);
        }
    }
};

}

namespace NeoWPF::UI {

void RegisterPanelStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(Panel), std::make_shared<Styling::PanelStyle>());
}

}
