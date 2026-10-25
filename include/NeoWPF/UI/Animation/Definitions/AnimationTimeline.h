/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Animation/Easing/EasingFunctionBase.h>

#include <any>
#include <memory>

namespace NeoWPF::UI::Animation {

/*
    @summary
    Defines common timing for an animation.
*/
enum class RepeatBehavior { Once, Loop, Count };
enum class FillBehavior { Stop, HoldEnd };

/*
    @summary
    Base class for value animations.
*/
class AnimationTimeline {
public:
    virtual ~AnimationTimeline() = default;
    virtual std::any GetCurrentValue(double progress) const = 0;

    /*
        @summary
        Sets the animation duration.

        @param seconds
        Duration in seconds.
    */
    void SetDuration(float seconds) noexcept { m_duration = seconds < 0.0f ? 0.0f : seconds; }

    /*
        @summary
        Sets the animation start delay.

        @param seconds
        Delay in seconds.
    */
    void SetBeginTime(float seconds) noexcept { m_beginTime = seconds < 0.0f ? 0.0f : seconds; }

    /*
        @summary
        Sets whether the animation reverses after each pass.

        @param value
        True to reverse automatically.
    */
    void SetAutoReverse(bool value) noexcept { m_autoReverse = value; }

    /*
        @summary
        Sets the animation repeat behavior.

        @param behavior
        Repeat behavior.

        @param count
        Repeat count when Count is selected.
    */
    void SetRepeatBehavior(RepeatBehavior behavior, int count = 1) noexcept;

    /*
        @summary
        Returns the current animation duration in seconds.
    */
    float GetDuration() const noexcept { return m_duration; }

    /*
        @summary
        Returns the current animation start delay in seconds.
    */
    float GetBeginTime() const noexcept { return m_beginTime; }

    /*
        @summary
        Returns whether the animation reverses automatically after each pass.
    */
    bool GetAutoReverse() const noexcept { return m_autoReverse; }

    /*
        @summary
        Returns the current repeat behavior.
    */
    RepeatBehavior GetRepeatBehavior() const noexcept { return m_repeatBehavior; }

    /*
        @summary
        Returns the current repeat count.
    */
    int GetRepeatCount() const noexcept { return m_repeatCount; }

    /*
        @summary
        Sets the behavior after the animation completes.

        @param behavior
        Fill behavior.
    */
    void SetFillBehavior(FillBehavior behavior) noexcept { m_fillBehavior = behavior; }

    /*
        @summary
        Sets the easing function.

        @param easing
        Easing function to use.
    */
    void SetEasingFunction(std::shared_ptr<Easing::EasingFunctionBase> easing) noexcept { m_easingFunction = std::move(easing); }

    /*
        @summary
        Returns the fill behavior after the animation completes.
    */
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