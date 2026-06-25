#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace NeoWPF::UI {
    class UIElement;
}

namespace NeoWPF::UI::Styling {

class Style;
using StylePtr = std::shared_ptr<Style>;

class StyleManager {
public:
    static void RegisterDefault(const std::type_index& type, const StylePtr& style);
    static StylePtr GetDefault(const std::type_index& type);

private:
    static std::unordered_map<std::type_index, StylePtr> s_defaults;
};

}
