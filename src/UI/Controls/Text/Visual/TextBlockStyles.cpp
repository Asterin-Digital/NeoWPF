#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::UI {

void ApplyTextBlockThemeStyle(UIElement* element)
{
    if (!element) {
        return;
    }
    element->SetStylePropertyValue("TextColor", uint32_t(0xFF000000u));
    element->SetStylePropertyValue("FontSize", element->GetResourceValue("TextBlock.FontSize", 12));
    const std::wstring defaultFontFamily = element->GetResourceValue<std::wstring>(
        "FontFamily", std::wstring(L"Segoe UI"));
    const std::wstring fontFamily = element->GetResourceValue<std::wstring>(
        "TextBlock.FontFamily",
        defaultFontFamily);
    element->SetStylePropertyValue("FontFamily", fontFamily);
    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Info,
        "ApplyTextBlockThemeStyle: FontFamily='%ls'",
        fontFamily.c_str());
}

}
