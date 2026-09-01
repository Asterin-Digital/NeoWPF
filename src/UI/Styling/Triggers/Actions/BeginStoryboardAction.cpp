#include <NeoWPF/UI/Styling/Triggers/Actions/BeginStoryboardAction.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Styling::Triggers {

BeginStoryboardAction::BeginStoryboardAction(std::shared_ptr<UI::Animation::Storyboard> storyboard)
    : m_storyboard(std::move(storyboard))
{
}

void BeginStoryboardAction::Invoke(UIElement* element) const {
    if (!element || !m_storyboard) {
        return;
    }

    NeoWPF::UI::BeginStoryboard(m_storyboard, element);
}

}
