#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Animation/AnimationController.h>

#include <unordered_map>

namespace NeoWPF::UI::Styling {

namespace detail {

namespace {

std::unordered_map<UIElement*, AnimationHandle>& ActiveTransitionHandles() {
    static std::unordered_map<UIElement*, AnimationHandle> handles;
    return handles;
}

}

bool ApplyVisualStateTransition(UIElement* element, const std::shared_ptr<VisualState>& state, bool useTransitions) {
    if (!element || !state) {
        return false;
    }

    auto& handles = ActiveTransitionHandles();
    if (const auto it = handles.find(element); it != handles.end()) {
        StopStoryboard(it->second);
        handles.erase(it);
    }

    if (useTransitions && state->HasStoryboard()) {
        const auto handle = BeginStoryboard(state->GetStoryboard(), element);
        if (handle != kInvalidAnimationHandle) {
            handles[element] = handle;
        }
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
