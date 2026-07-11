#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class ButtonStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        element->SetStylePropertyValue("TextColor", uint32_t(0xFFEDEDEDu));
        element->SetStylePropertyValue("FontSize", 14);
        element->SetStylePropertyValue("Padding", 10);
        element->SetStylePropertyValue("BackgroundColor", uint32_t(0xFF3A3A3Au));

        element->SetStyleProperty("VisualState.Normal.Background", 0xFF3A3A3Au);
        element->SetStyleProperty("VisualState.MouseOver.Background", 0xFF5A7A9Au);
        element->SetStyleProperty("VisualState.Pressed.Background", 0xFF1A1A1Au);
        element->SetStyleProperty("VisualState.Disabled.Background", 0xFF2A2A2Au);
    }
};

}

namespace NeoWPF::UI {

void RegisterButtonStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(Button), std::make_shared<Styling::ButtonStyle>());
}

}
