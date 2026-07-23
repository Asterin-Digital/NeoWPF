#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Clock.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Diagnostics/Logger.h>

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
    NeoWPF::Diagnostics::LogInfo("[Animation] queued storyboard count=%zu", GetAnimations().size());
}

void Tick() {
    auto& animations = GetAnimations();
    for (auto& animation : animations) {
        if (!animation.active || !animation.storyboard) {
            continue;
        }

        const float elapsed = animation.clock.Tick();
        const bool completed = animation.storyboard->Update(elapsed);
        animation.active = !completed;
        if (completed) {
            NeoWPF::Diagnostics::LogInfo("[Animation] storyboard completed");
        }
    }

    RemoveCompletedAnimations();
}

}
