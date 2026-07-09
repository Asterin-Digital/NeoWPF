#pragma once

#include <string>
#include <typeindex>
#include <any>
#include <functional>

namespace NeoWPF::UI {
    class UIElement;
}

namespace NeoWPF::UI::Properties {
    struct PropertyInfo {
        std::string Name;
        std::type_index Type;
        std::function<void(UIElement*, const std::any&)> Setter;
        std::function<std::any(UIElement*)> Getter;
    };
}
