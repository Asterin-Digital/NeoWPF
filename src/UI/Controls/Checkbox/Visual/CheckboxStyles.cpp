#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class CheckboxStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        element->SetStylePropertyValue("TextColor", NeoWPF::UI::Color::FromArgb(0xFFEDEDEDu));
        element->SetStylePropertyValue("BoxColor", NeoWPF::UI::Color::FromArgb(0xFF3A3A3Au));
        element->SetStylePropertyValue("BorderColor", NeoWPF::UI::Color::FromArgb(0xFF666666u));
    }
};

}

namespace NeoWPF::UI {

void RegisterCheckboxStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(Checkbox), std::make_shared<Styling::CheckboxStyle>());
}

}
