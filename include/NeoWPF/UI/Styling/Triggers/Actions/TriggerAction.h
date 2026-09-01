#pragma once

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Styling::Triggers {

class TriggerAction {
public:
    virtual ~TriggerAction() = default;
    virtual void Invoke(UIElement* element) const = 0;
};

}
