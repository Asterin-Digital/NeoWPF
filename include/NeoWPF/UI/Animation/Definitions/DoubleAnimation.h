/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Animation/Definitions/AnimationTimeline.h>

namespace NeoWPF::UI::Animation {

/*
    @summary
    Animates a floating point value.
*/
class DoubleAnimation final : public AnimationTimeline {
public:
    /*
        @summary
        Sets the starting value.
    */
    void SetFrom(float value) noexcept { m_from = value; }

    /*
        @summary
        Returns the starting value of the animation.
    */
    float GetFrom() const noexcept { return m_from; }

    /*
        @summary
        Sets the ending value.
    */
    void SetTo(float value) noexcept { m_to = value; }

    /*
        @summary
        Returns the ending value of the animation.
    */
    float GetTo() const noexcept { return m_to; }
    std::any GetCurrentValue(double progress) const override;

private:
    float m_from = 0.0f;
    float m_to = 0.0f;
};

}