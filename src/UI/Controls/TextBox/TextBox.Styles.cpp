#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
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
            textBox->SetCaretColor(0xFFFFFFFFu);
            textBox->SetFontSize(14);
            textBox->SetPadding(4);

            textBox->SetProperty("VisualState.Normal.Background", 0xFF2F2F2Fu);
            textBox->SetProperty("VisualState.Normal.Border", 0xFF666666u);
            textBox->SetProperty("VisualState.Focused.Background", 0xFF1F1F1Fu);
            textBox->SetProperty("VisualState.Focused.Border", 0xFF0078D4u);
            textBox->SetProperty("VisualState.Disabled.Background", 0xFF202020u);
            textBox->SetProperty("VisualState.Disabled.Border", 0xFF404040u);
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
