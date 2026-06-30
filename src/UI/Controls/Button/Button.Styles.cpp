#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Styling/StyleManager.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class ButtonStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        if (auto button = dynamic_cast<UI::Button*>(element)) {
            button->SetTextColor(0xFF212121u);
            button->SetFontSize(14);
            button->SetPadding(10);
            button->SetBackgroundColor(0xFFF0F0F0u);
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
