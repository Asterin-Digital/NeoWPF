#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Styling/StyleManager.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class TextBlockStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        if (auto textBlock = dynamic_cast<UI::TextBlock*>(element)) {
            textBlock->SetTextColor(0xFF212121u);
            textBlock->SetFontSize(12);
        }
    }
};

}

namespace NeoWPF::UI {

void RegisterTextBlockStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(TextBlock), std::make_shared<Styling::TextBlockStyle>());
}

}
