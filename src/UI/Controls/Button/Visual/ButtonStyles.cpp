#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class ButtonStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        element->SetPropertyValue("TextColor", uint32_t(0xFFEDEDEDu));
        element->SetPropertyValue("FontSize", 14);
        element->SetPropertyValue("Padding", 10);
        element->SetPropertyValue("BackgroundColor", uint32_t(0xFF3A3A3Au));

        element->SetProperty("VisualState.Normal.Background", 0xFF3A3A3Au);
        element->SetProperty("VisualState.MouseOver.Background", 0xFF5A7A9Au);
        element->SetProperty("VisualState.Pressed.Background", 0xFF1A1A1Au);
        element->SetProperty("VisualState.Disabled.Background", 0xFF2A2A2Au);
    }
};

}

namespace NeoWPF::UI {

void RegisterButtonStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(Button), std::make_shared<Styling::ButtonStyle>());
}

}
