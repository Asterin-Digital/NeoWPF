/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoWPF::UI::Animation::Easing {

/*
    @summary
    Defines how an animation progresses over time.
*/
enum class EasingMode {
    In,
    Out,
    InOut,
};

/*
    @summary
    Base class for easing functions.
*/
class EasingFunctionBase {
public:
    explicit EasingFunctionBase(EasingMode mode = EasingMode::In) noexcept
        : m_mode(mode) {}

    virtual ~EasingFunctionBase() = default;

    /*
        @summary
        Applies the configured easing mode to normalized time.

        @param normalizedTime
        Input time between zero and one.
    */
    double Ease(double normalizedTime) const noexcept;
    /*
        @summary
        Returns the configured easing mode.
    */
    EasingMode GetEasingMode() const noexcept { return m_mode; }
    /*
        @summary
        Sets the easing mode.

        @param mode
        Easing mode to apply.
    */
    void SetEasingMode(EasingMode mode) noexcept { m_mode = mode; }

protected:
    virtual double EaseIn(double normalizedTime) const noexcept = 0;

private:
    EasingMode m_mode;
};

}
