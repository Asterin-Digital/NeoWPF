#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <typeindex>

namespace NeoWPF::UI::Styling {

class TextBoxStyle : public Style {
public:
    void Apply(UIElement* element) override
    {
        element->SetPropertyValue("TextColor", uint32_t(0xFFEDEDEDu));
        element->SetPropertyValue("BackgroundColor", uint32_t(0xFF2F2F2Fu));
        element->SetPropertyValue("BorderColor", uint32_t(0xFF666666u));
        element->SetPropertyValue("CaretColor", uint32_t(0xFFFFFFFFu));
        element->SetPropertyValue("FontSize", 14);
        element->SetPropertyValue("Padding", 4);

        element->SetProperty("VisualState.Normal.Background", 0xFF2F2F2Fu);
        element->SetProperty("VisualState.Normal.Border", 0xFF666666u);
        element->SetProperty("VisualState.Focused.Background", 0xFF1F1F1Fu);
        element->SetProperty("VisualState.Focused.Border", 0xFF0078D4u);
        element->SetProperty("VisualState.Disabled.Background", 0xFF202020u);
        element->SetProperty("VisualState.Disabled.Border", 0xFF404040u);
    }
};

}

namespace NeoWPF::UI {

void RegisterTextBoxStyles()
{
    Styling::StyleManager::RegisterDefault(typeid(TextBox), std::make_shared<Styling::TextBoxStyle>());
}

}
