#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Clock.h>
#include <NeoWPF/UI/Animation/Core/AnimationClock.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef animationDurationSeconds
#undef animationDurationSeconds
#endif
#ifdef elapsedSeconds
#undef elapsedSeconds
#endif

namespace NeoWPF::UI {
namespace {

struct ActiveAnimation {
    AnimationHandle handle = kInvalidAnimationHandle;
    ::NeoWPF::UI::Animation::Clock clock;
    std::vector<::NeoWPF::UI::Animation::AnimationClock> clocks;
    std::function<void(float)> valueCallback;
    float valueFrom = 0.0f;
    float valueTo = 0.0f;
    float callbackElapsed = 0.0f;
    float callbackDuration = 0.0f;
};

std::unordered_map<AnimationHandle, ActiveAnimation>& GetAnimations() {
    static std::unordered_map<AnimationHandle, ActiveAnimation> animations;
    return animations;
}

AnimationHandle& NextHandle() {
    static AnimationHandle nextHandle = 1;
    return nextHandle;
}

void RemoveCompletedAnimations() {
    auto& animations = GetAnimations();
    for (auto it = animations.begin(); it != animations.end();) {
        const bool completed = it->second.valueCallback
            ? it->second.callbackElapsed >= it->second.callbackDuration
            : std::all_of(it->second.clocks.begin(), it->second.clocks.end(), [](const auto& clock) {
                return clock.IsFinished();
            });
        if (completed) {
            it = animations.erase(it);
        } else {
            ++it;
        }
    }
}

}

AnimationHandle BeginValueAnimation(float fromValue,
                                    float toValue,
                                    float animationDurationSeconds,
                                    std::function<void(float)> callback) {
    if (!callback) {
        return kInvalidAnimationHandle;
    }

    ActiveAnimation animation;
    animation.handle = NextHandle()++;
    animation.valueCallback = std::move(callback);
    animation.valueFrom = fromValue;
    animation.valueTo = toValue;
    animation.callbackDuration = animationDurationSeconds > 0.0f ? animationDurationSeconds : 0.0f;
    if (animation.callbackDuration == 0.0f) {
        animation.valueCallback(toValue);
        return kInvalidAnimationHandle;
    }

    const auto handle = animation.handle;
    GetAnimations().emplace(handle, std::move(animation));
    return handle;
}

void StopAnimation(AnimationHandle handle) {
    if (handle == kInvalidAnimationHandle) {
        return;
    }
    GetAnimations().erase(handle);
}

AnimationHandle BeginStoryboard(std::shared_ptr<::NeoWPF::UI::Animation::Storyboard> storyboard, UIElement* target) {
    if (!storyboard || !target) {
        return kInvalidAnimationHandle;
    }

    ActiveAnimation animation;
    animation.handle = NextHandle()++;
    animation.clock.Reset();
    for (const auto& child : storyboard->GetChildren()) {
        UIElement* childTarget = target;
        if (!child.TargetName.empty()) {
            const auto childVisual = target->FindTemplateChild(child.TargetName);
            childTarget = childVisual ? dynamic_cast<UIElement*>(childVisual.get()) : nullptr;
        }
        if (!childTarget) {
            continue;
        }
        const auto* property = childTarget->FindDependencyProperty(child.TargetPropertyName);
        if (property && child.Timeline) {
            animation.clocks.emplace_back(childTarget, property, child.Timeline);
        }
    }

    if (animation.clocks.empty()) {
        return kInvalidAnimationHandle;
    }
    const auto handle = animation.handle;
    GetAnimations().emplace(handle, std::move(animation));
    NeoWPF::Diagnostics::LogInfo("[Animation] started storyboard handle=%llu count=%zu",
                                 static_cast<unsigned long long>(handle), GetAnimations().size());
    return handle;
}

void StopStoryboard(AnimationHandle handle) {
    if (handle == kInvalidAnimationHandle) {
        return;
    }
    auto it = GetAnimations().find(handle);
    if (it == GetAnimations().end()) {
        return;
    }
    for (auto& clock : it->second.clocks) {
        clock.Stop();
    }
    GetAnimations().erase(it);
}

void Tick() {
    auto& animations = GetAnimations();
    for (auto& [handle, animation] : animations) {
        const float elapsedSeconds = animation.clock.Tick();
        if (animation.valueCallback) {
            animation.callbackElapsed += elapsedSeconds;
            if (animation.callbackElapsed > animation.callbackDuration) {
                animation.callbackElapsed = animation.callbackDuration;
            }
            const float linearProgress = animation.callbackElapsed / animation.callbackDuration;
            const float smoothProgress = linearProgress * linearProgress * (3.0f - 2.0f * linearProgress);
            animation.valueCallback(animation.valueFrom + (animation.valueTo - animation.valueFrom) * smoothProgress);
            continue;
        }
        for (auto& clock : animation.clocks) {
            if (!clock.IsFinished() && clock.Tick(elapsedSeconds)) {
                NeoWPF::Diagnostics::LogInfo("[Animation] child completed handle=%llu",
                                             static_cast<unsigned long long>(handle));
            }
        }
    }

    RemoveCompletedAnimations();
}

}
