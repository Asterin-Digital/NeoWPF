#pragma once

#include <NeoWPF/UI/Animation/Definitions/AnimationTimeline.h>

namespace NeoWPF::UI::Animation {

class DoubleAnimation final : public AnimationTimeline {
public:
    void SetFrom(float value) noexcept { m_from = value; }
    void SetTo(float value) noexcept { m_to = value; }
    float GetFrom() const noexcept { return m_from; }
    float GetTo() const noexcept { return m_to; }
    std::any GetCurrentValue(double progress) const override;

private:
    float m_from = 0.0f;
    float m_to = 0.0f;
};

}