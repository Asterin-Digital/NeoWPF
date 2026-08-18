#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void ApplyTextBoxThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }

    element->SetStylePropertyValue("TextColor", element->GetResourceValue("TextBox.TextColor", uint32_t(0xFF000000u)));
    element->SetStylePropertyValue("BackgroundColor", element->GetResourceValue("TextBox.BackgroundColor", uint32_t(0xFFFFFFFFu)));
    element->SetStylePropertyValue("BorderColor", element->GetResourceValue("TextBox.BorderColor", uint32_t(0xFFABADB3u)));
    element->SetStylePropertyValue("CaretColor", element->GetResourceValue("TextBox.CaretColor", uint32_t(0xFF000000u)));
    element->SetStylePropertyValue("FontSize", element->GetResourceValue("TextBox.FontSize", 14));
    const std::wstring defaultFontFamily = element->GetResourceValue<std::wstring>(
        "FontFamily", std::wstring());
    const std::wstring fontFamily = element->GetResourceValue<std::wstring>(
        "TextBox.FontFamily", defaultFontFamily);
    element->SetStylePropertyValue("FontFamily", fontFamily);
    element->SetStylePropertyValue("Padding", element->GetResourceValue("TextBox.Padding", 4));

    element->SetStyleProperty("VisualState.Normal.Background", element->GetResourceValue("TextBox.VisualState.Normal.Background", 0xFFFFFFFFu));
    element->SetStyleProperty("VisualState.Normal.Border", element->GetResourceValue("TextBox.VisualState.Normal.Border", 0xFFABADB3u));
    element->SetStyleProperty("VisualState.Focused.Background", element->GetResourceValue("TextBox.VisualState.Focused.Background", 0xFFFFFFFFu));
    element->SetStyleProperty("VisualState.Focused.Border", element->GetResourceValue("TextBox.VisualState.Focused.Border", 0xFF569DE5u));
    element->SetStyleProperty("VisualState.Disabled.Background", element->GetResourceValue("TextBox.VisualState.Disabled.Background", 0xFFF0F0F0u));
    element->SetStyleProperty("VisualState.Disabled.Border", element->GetResourceValue("TextBox.VisualState.Disabled.Border", 0xFFABADB3u));
}

}
