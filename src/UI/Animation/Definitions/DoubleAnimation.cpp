#include <NeoWPF/UI/Animation/Definitions/DoubleAnimation.h>

namespace NeoWPF::UI::Animation {

std::any DoubleAnimation::GetCurrentValue(double progress) const {
    const double easedProgress = ApplyEasing(progress);
    return static_cast<float>(m_from + (m_to - m_from) * easedProgress);
}

}
