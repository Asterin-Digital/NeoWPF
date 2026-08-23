#pragma once

#include <NeoWPF/UI/Animation/Easing/EasingFunctionBase.h>

#include <any>
#include <memory>

namespace NeoWPF::UI::Animation {

enum class RepeatBehavior { Once, Loop, Count };
enum class FillBehavior { Stop, HoldEnd };

class AnimationTimeline {
public:
    virtual ~AnimationTimeline() = default;
    virtual std::any GetCurrentValue(double progress) const = 0;

    void SetDuration(float seconds) noexcept { m_duration = seconds < 0.0f ? 0.0f : seconds; }
    void SetBeginTime(float seconds) noexcept { m_beginTime = seconds < 0.0f ? 0.0f : seconds; }
    void SetAutoReverse(bool value) noexcept { m_autoReverse = value; }
    void SetRepeatBehavior(RepeatBehavior behavior, int count = 1) noexcept;
    void SetFillBehavior(FillBehavior behavior) noexcept { m_fillBehavior = behavior; }
    void SetEasingFunction(std::shared_ptr<Easing::EasingFunctionBase> easing) noexcept { m_easingFunction = std::move(easing); }

    float GetDuration() const noexcept { return m_duration; }
    float GetBeginTime() const noexcept { return m_beginTime; }
    bool GetAutoReverse() const noexcept { return m_autoReverse; }
    RepeatBehavior GetRepeatBehavior() const noexcept { return m_repeatBehavior; }
    int GetRepeatCount() const noexcept { return m_repeatCount; }
    FillBehavior GetFillBehavior() const noexcept { return m_fillBehavior; }

protected:
    double ApplyEasing(double linearProgress) const noexcept;

private:
    float m_duration = 0.0f;
    float m_beginTime = 0.0f;
    bool m_autoReverse = false;
    RepeatBehavior m_repeatBehavior = RepeatBehavior::Once;
    int m_repeatCount = 1;
    FillBehavior m_fillBehavior = FillBehavior::Stop;
    std::shared_ptr<Easing::EasingFunctionBase> m_easingFunction;
};

}