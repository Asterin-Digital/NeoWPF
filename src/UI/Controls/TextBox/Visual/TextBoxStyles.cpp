#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class TextBoxStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        element->SetStylePropertyValue("TextColor", element->GetResourceValue("TextBox.TextColor", uint32_t(0xFFEDEDEDu)));
        element->SetStylePropertyValue("BackgroundColor", element->GetResourceValue("TextBox.BackgroundColor", uint32_t(0xFF2F2F2Fu)));
        element->SetStylePropertyValue("BorderColor", element->GetResourceValue("TextBox.BorderColor", uint32_t(0xFF666666u)));
        element->SetStylePropertyValue("CaretColor", element->GetResourceValue("TextBox.CaretColor", uint32_t(0xFFFFFFFFu)));
        element->SetStylePropertyValue("FontSize", element->GetResourceValue("TextBox.FontSize", 14));
        element->SetStylePropertyValue("Padding", element->GetResourceValue("TextBox.Padding", 4));

        element->SetStyleProperty("VisualState.Normal.Background", element->GetResourceValue("TextBox.VisualState.Normal.Background", 0xFF2F2F2Fu));
        element->SetStyleProperty("VisualState.Normal.Border", element->GetResourceValue("TextBox.VisualState.Normal.Border", 0xFF666666u));
        element->SetStyleProperty("VisualState.Focused.Background", element->GetResourceValue("TextBox.VisualState.Focused.Background", 0xFF1F1F1Fu));
        element->SetStyleProperty("VisualState.Focused.Border", element->GetResourceValue("TextBox.VisualState.Focused.Border", 0xFF0078D4u));
        element->SetStyleProperty("VisualState.Disabled.Background", element->GetResourceValue("TextBox.VisualState.Disabled.Background", 0xFF202020u));
        element->SetStyleProperty("VisualState.Disabled.Border", element->GetResourceValue("TextBox.VisualState.Disabled.Border", 0xFF404040u));
    }
};

}

namespace NeoWPF::UI {

void RegisterTextBoxStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(TextBox), std::make_shared<Styling::TextBoxStyle>());
}

}
