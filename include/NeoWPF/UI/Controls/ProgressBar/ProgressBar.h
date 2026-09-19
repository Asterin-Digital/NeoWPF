/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>
#include <cstdint>
#include <memory>

namespace NeoWPF::UI {

class ProgressBarTemplate : public Templates::ControlTemplate {
public:
    ProgressBarTemplate();
};

class ProgressBar : public RangeBase {
public:
    ProgressBar();
    ~ProgressBar() override;

    void SetOrientation(Orientation orientation) noexcept;
    Orientation GetOrientation() const noexcept { return m_orientation; }

    void SetIsIndeterminate(bool value) noexcept;
    bool GetIsIndeterminate() const noexcept { return m_isIndeterminate; }

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;
    void OnInitialize() override;

    static const Properties::DependencyProperty& GetOrientationProperty();
    static const Properties::DependencyProperty& IsIndeterminateProperty();
    static const Properties::DependencyProperty& AnimationProgressProperty();
    static const Properties::DependencyProperty& IndeterminatePhaseProperty();

    int GetOrientationValue() const noexcept;
    void SetOrientationValue(int orientation) noexcept;
    void SetAnimationProgress(float value) noexcept;
    float GetAnimationProgress() const noexcept { return m_animationProgress; }
    void SetIndeterminatePhase(float value) noexcept;
    float GetIndeterminatePhase() const noexcept { return m_indeterminatePhase; }

protected:
    void OnValueChanged(double value) override;

private:
    void StartDeterminateAnimation() noexcept;
    void StartIndeterminateAnimation() noexcept;
    void StopAnimations() noexcept;
    void UpdateVisualState() noexcept;
    double GetProgressFraction() const noexcept;

    Orientation m_orientation = Orientation::Horizontal;
    bool m_isIndeterminate = false;
    float m_animationProgress = 0.0f;
    float m_indeterminatePhase = 0.0f;
    AnimationHandle m_determinateAnimation = kInvalidAnimationHandle;
    AnimationHandle m_indeterminateAnimation = kInvalidAnimationHandle;
    bool m_isInitialized = false;
};

}
