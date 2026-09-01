#include <NeoWPF/UI/Styling/Triggers/Actions/StopStoryboardAction.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Styling::Triggers {

StopStoryboardAction::StopStoryboardAction(std::uint64_t animationHandle)
    : m_animationHandle(animationHandle)
{
}

void StopStoryboardAction::Invoke(UIElement* element) const {
    if (!element || m_animationHandle == 0) {
        return;
    }

    NeoWPF::UI::StopStoryboard(m_animationHandle);
}

}
