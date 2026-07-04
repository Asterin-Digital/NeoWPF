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
