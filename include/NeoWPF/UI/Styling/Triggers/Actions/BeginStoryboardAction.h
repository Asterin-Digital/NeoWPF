#pragma once

#include <memory>

#include <NeoWPF/UI/Styling/Triggers/Actions/TriggerAction.h>

namespace NeoWPF::UI::Animation { class Storyboard; }

namespace NeoWPF::UI::Styling::Triggers {

class BeginStoryboardAction : public TriggerAction {
public:
    explicit BeginStoryboardAction(std::shared_ptr<UI::Animation::Storyboard> storyboard);
    void Invoke(UIElement* element) const override;

private:
    std::shared_ptr<UI::Animation::Storyboard> m_storyboard;
};

}
