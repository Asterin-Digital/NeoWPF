#pragma once

#include <memory>

namespace NeoWPF::UI {
    class UIElement;
}

namespace NeoWPF::UI::Styling {

class Style {
public:
    virtual ~Style() = default;
    virtual void Apply(NeoWPF::UI::UIElement* element) {}
};

using StylePtr = std::shared_ptr<Style>;

}
