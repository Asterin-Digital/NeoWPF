#pragma once

#include <NeoWPF/UI/Animation/Definitions/AnimationTimeline.h>
#include <NeoWPF/UI/Core/Media/Color.h>

namespace NeoWPF::UI::Animation {

class ColorAnimation final : public AnimationTimeline {
public:
    void SetFrom(const NeoWPF::UI::Color& value) noexcept { m_from = value; }
    void SetTo(const NeoWPF::UI::Color& value) noexcept { m_to = value; }
    const NeoWPF::UI::Color& GetFrom() const noexcept { return m_from; }
    const NeoWPF::UI::Color& GetTo() const noexcept { return m_to; }
    std::any GetCurrentValue(double progress) const override;

private:
    NeoWPF::UI::Color m_from{};
    NeoWPF::UI::Color m_to{};
};

}