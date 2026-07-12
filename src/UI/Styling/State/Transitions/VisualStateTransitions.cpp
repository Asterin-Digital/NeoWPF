#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Animation/AnimationController.h>

namespace NeoWPF::UI::Styling {

namespace detail {

bool ApplyVisualStateTransition(UIElement* element, const std::shared_ptr<VisualState>& state, bool useTransitions) {
    if (!element || !state) {
        return false;
    }

    if (useTransitions && state->HasStoryboard()) {
        AddStoryboard(state->GetStoryboard());
    }

    for (const auto& setter : state->GetSetters()) {
        if (setter.TargetName.empty()) {
            continue;
        }

        if (auto target = element->FindTemplateChild(setter.TargetName)) {
            setter.Callback(element, target.get());
        }
    }

    return true;
}

}

}
