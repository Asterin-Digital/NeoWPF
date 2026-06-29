#include <NeoWPF/UI/Styling/StyleManager.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
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

class ButtonStyle : public Style {
public:
    void Apply(NeoWPF::UI::UIElement* element) override
    {
        if (auto button = dynamic_cast<NeoWPF::UI::Button*>(element)) {
            button->SetTextColor(0xFF000000u);
            button->SetFontSize(14);
            button->SetPadding(8);
            button->SetBackgroundColor(0xFFC0C0C0u);
        }
    }
};

void RegisterDefaultStyles()
{
    StyleManager::RegisterDefault(typeid(NeoWPF::UI::Panel), std::make_shared<PanelStyle>());
    StyleManager::RegisterDefault(typeid(NeoWPF::UI::Button), std::make_shared<ButtonStyle>());
}

}
