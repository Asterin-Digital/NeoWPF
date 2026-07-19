#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Styling {

std::unordered_map<std::type_index, StylePtr> StyleManager::s_defaults;

void StyleManager::RegisterDefault(const std::type_index& type, const StylePtr& style)
{
    if (!style) return;
    s_defaults[type] = style;
}

StylePtr StyleManager::GetDefault(const std::type_index& type)
{
    auto it = s_defaults.find(type);
    return it != s_defaults.end() ? it->second : nullptr;
}

void ReapplyDefaultStyleForElement(UIElement* element)
{
    if (!element) {
        return;
    }

    if (auto style = StyleManager::GetDefault(typeid(*element))) {
        style->Apply(element);
    }
}

}
