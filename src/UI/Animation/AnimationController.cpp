#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Clock.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace NeoWPF::UI {
namespace {

struct ActiveAnimation {
    std::shared_ptr<::NeoWPF::UI::Animation::Storyboard> storyboard;
    ::NeoWPF::UI::Animation::Clock clock;
    bool active = false;
};

std::vector<ActiveAnimation>& GetAnimations() {
    static std::vector<ActiveAnimation> animations;
    return animations;
}

void RemoveCompletedAnimations() {
    auto& animations = GetAnimations();
    animations.erase(std::remove_if(animations.begin(), animations.end(), [](const ActiveAnimation& animation) {
        return !animation.active;
    }), animations.end());
}

}

void AddStoryboard(std::shared_ptr<::NeoWPF::UI::Animation::Storyboard> storyboard) {
    if (!storyboard) return;
    ActiveAnimation animation;
    animation.storyboard = std::move(storyboard);
    animation.active = true;
    GetAnimations().push_back(std::move(animation));
}

// Element-level BeginAnimation functions are implemented as member methods on the element types.

void Tick() {
    auto& animations = GetAnimations();
    for (auto& animation : animations) {
        if (!animation.active || !animation.storyboard) {
            continue;
        }

        const float elapsed = animation.clock.Tick();
        animation.active = !animation.storyboard->Update(elapsed);
    }

    RemoveCompletedAnimations();
}

}
