#pragma once

#include <NeoWPF/UI/Animation/Easing/EasingFunctionBase.h>

namespace NeoWPF::UI::Animation::Easing {

class QuadraticEase final : public EasingFunctionBase {
public:
    using EasingFunctionBase::EasingFunctionBase;

protected:
    double EaseIn(double normalizedTime) const noexcept override;
};

class CubicEase final : public EasingFunctionBase {
public:
    using EasingFunctionBase::EasingFunctionBase;

protected:
    double EaseIn(double normalizedTime) const noexcept override;
};

class SineEase final : public EasingFunctionBase {
public:
    using EasingFunctionBase::EasingFunctionBase;

protected:
    double EaseIn(double normalizedTime) const noexcept override;
};

class CircleEase final : public EasingFunctionBase {
public:
    using EasingFunctionBase::EasingFunctionBase;

protected:
    double EaseIn(double normalizedTime) const noexcept override;
};

}
