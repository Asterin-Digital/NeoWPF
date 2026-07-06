#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class ButtonStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        if (auto button = dynamic_cast<UI::Button*>(element)) {
            button->SetTextColor(0xFFEDEDEDu);
            button->SetFontSize(14);
            button->SetPadding(10);
            button->SetBackgroundColor(0xFF3A3A3Au);

            button->SetProperty("VisualState.Normal.Background", 0xFF3A3A3Au);
            button->SetProperty("VisualState.MouseOver.Background", 0xFF5A7A9Au);
            button->SetProperty("VisualState.Pressed.Background", 0xFF1A1A1Au);
            button->SetProperty("VisualState.Disabled.Background", 0xFF2A2A2Au);
        }
    }
};

}

namespace NeoWPF::UI {

void RegisterButtonStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(Button), std::make_shared<Styling::ButtonStyle>());
}

}
