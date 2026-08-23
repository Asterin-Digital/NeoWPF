#include <NeoWPF/UI/Animation/Definitions/AnimationTimeline.h>

#include <algorithm>

namespace NeoWPF::UI::Animation {

void AnimationTimeline::SetRepeatBehavior(RepeatBehavior behavior, int count) noexcept {
    m_repeatBehavior = behavior;
    m_repeatCount = std::max(1, count);
}

double AnimationTimeline::ApplyEasing(double linearProgress) const noexcept {
    const double progress = std::clamp(linearProgress, 0.0, 1.0);
    return m_easingFunction ? m_easingFunction->Ease(progress) : progress;
}

}
