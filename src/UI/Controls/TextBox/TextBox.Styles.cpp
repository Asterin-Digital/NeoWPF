#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Styling/StyleManager.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class TextBoxStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        if (auto textBox = dynamic_cast<UI::TextBox*>(element)) {
            textBox->SetTextColor(0xFFEDEDEDu);
            textBox->SetBackgroundColor(0xFF2F2F2Fu);
            textBox->SetBorderColor(0xFF666666u);
            textBox->SetCaretColor(0xFFEDEDEDu);
            textBox->SetFontSize(14);
            textBox->SetPadding(4);
        }
    }
};

}

namespace NeoWPF::UI {

void RegisterTextBoxStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(TextBox), std::make_shared<Styling::TextBoxStyle>());
}

}
