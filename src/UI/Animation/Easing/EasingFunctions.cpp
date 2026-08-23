#include <NeoWPF/UI/Animation/Easing/EasingFunctions.h>

#include <cmath>

namespace NeoWPF::UI::Animation::Easing {

double QuadraticEase::EaseIn(double normalizedTime) const noexcept {
    return normalizedTime * normalizedTime;
}

double CubicEase::EaseIn(double normalizedTime) const noexcept {
    return normalizedTime * normalizedTime * normalizedTime;
}

double SineEase::EaseIn(double normalizedTime) const noexcept {
    return 1.0 - std::cos(normalizedTime * 3.14159265358979323846 * 0.5);
}

double CircleEase::EaseIn(double normalizedTime) const noexcept {
    return 1.0 - std::sqrt(1.0 - normalizedTime * normalizedTime);
}

}
