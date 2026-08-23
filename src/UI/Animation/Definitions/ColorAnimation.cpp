#include <NeoWPF/UI/Animation/Definitions/ColorAnimation.h>

#include <algorithm>
#include <cmath>

namespace NeoWPF::UI::Animation {

namespace {

std::uint8_t InterpolateChannel(std::uint8_t from, std::uint8_t to, double progress) noexcept {
    const double value = static_cast<double>(from) +
                         (static_cast<double>(to) - static_cast<double>(from)) * progress;
    return static_cast<std::uint8_t>(std::clamp(std::lround(value), 0L, 255L));
}

}

std::any ColorAnimation::GetCurrentValue(double progress) const {
    const double easedProgress = ApplyEasing(progress);
    return NeoWPF::UI::Color(
        InterpolateChannel(m_from.A, m_to.A, easedProgress),
        InterpolateChannel(m_from.R, m_to.R, easedProgress),
        InterpolateChannel(m_from.G, m_to.G, easedProgress),
        InterpolateChannel(m_from.B, m_to.B, easedProgress));
}

}
