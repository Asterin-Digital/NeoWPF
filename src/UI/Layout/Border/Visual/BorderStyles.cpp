#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class BorderStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        if (auto border = dynamic_cast<UI::Border*>(element)) {
            border->SetPadding(0);
            border->SetBorderThickness(1);
            border->SetBorderColor(0xFF333333u);
            border->SetBackgroundColor(0xFF1E1E1Eu);
        }
    }
};

}

namespace NeoWPF::UI {

void RegisterBorderStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(Border), std::make_shared<Styling::BorderStyle>());
}

}
