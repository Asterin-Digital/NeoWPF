#include <NeoWPF/UI/Styling/StyleManager.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class PanelStyle : public Style {
public:
    void Apply(NeoWPF::UI::UIElement* element) override
    {
        if (auto panel = dynamic_cast<NeoWPF::UI::Panel*>(element)) {
            panel->SetBackgroundColor(0xFF1E88E5u);
        }
    }
};

void RegisterDefaultStyles()
{
    StyleManager::RegisterDefault(typeid(NeoWPF::UI::Panel), std::make_shared<PanelStyle>());
}

}
