#include <NeoWPF/UI/Animation/Easing/EasingFunctionBase.h>

#include <algorithm>

namespace NeoWPF::UI::Animation::Easing {

namespace {

double ClampProgress(double value) noexcept {
    return std::clamp(value, 0.0, 1.0);
}

}

double EasingFunctionBase::Ease(double normalizedTime) const noexcept {
    const double time = ClampProgress(normalizedTime);
    switch (m_mode) {
    case EasingMode::Out:
        return 1.0 - EaseIn(1.0 - time);
    case EasingMode::InOut:
        return time < 0.5
            ? EaseIn(time * 2.0) * 0.5
            : 1.0 - EaseIn((1.0 - time) * 2.0) * 0.5;
    case EasingMode::In:
    default:
        return EaseIn(time);
    }
}

}
