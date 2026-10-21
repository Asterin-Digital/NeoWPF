/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <cstddef>

namespace NeoWPF::UI {

class ProgressRing : public RangeBase {
public:
    ProgressRing();
    ~ProgressRing() override;

    /*
        @summary
        Sets whether the ring displays an animated indeterminate segment.

        @param value
        True to animate an indeterminate segment; otherwise, display the range value.
    */
    void SetIsIndeterminate(bool value) noexcept;

    /*
        @summary
        Gets whether the ring displays an animated indeterminate segment.
    */
    bool GetIsIndeterminate() const noexcept { return m_isIndeterminate; }

    /*
        @summary
        Sets the radial thickness of the ring in logical pixels.

        @param thickness
        The requested ring thickness; negative values become zero.
    */
    void SetRingThickness(double thickness) noexcept;

    /*
        @summary
        Gets the radial thickness of the ring in logical pixels.
    */
    double GetRingThickness() const noexcept { return m_ringThickness; }

    /*
        @summary
        Measures the ring using a square default size.

        @param availableWidth
        The width available to the ring.

        @param availableHeight
        The height available to the ring.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Arranges the ring in its final layout bounds.

        @param finalWidth
        The width assigned to the ring.

        @param finalHeight
        The height assigned to the ring.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

    /*
        @summary
        Adds the track and progress arc commands for the ring.

        @param commands
        The frame command list receiving the ring geometry.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    /*
        @summary
        Starts ring animation after the element joins the visual tree.

        @internal
    */
    void OnInitialize() override;

    static const Properties::DependencyProperty& IsIndeterminateProperty();
    static const Properties::DependencyProperty& RingThicknessProperty();
    static const Properties::DependencyProperty& AnimationProgressProperty();
    static const Properties::DependencyProperty& IndeterminatePhaseProperty();

    /*
        @summary
        Sets the animated determinate progress fraction.

        @param value
        The progress fraction, clamped to the unit interval.

        @internal
    */
    void SetAnimationProgress(float value) noexcept;

    /*
        @summary
        Gets the animated determinate progress fraction.

        @internal
    */
    float GetAnimationProgress() const noexcept { return m_animationProgress; }

    /*
        @summary
        Sets the animated indeterminate phase.

        @param value
        The phase, wrapped into the unit interval.

        @internal
    */
    void SetIndeterminatePhase(float value) noexcept;

    /*
        @summary
        Gets the animated indeterminate phase.

        @internal
    */
    float GetIndeterminatePhase() const noexcept { return m_indeterminatePhase; }

protected:
    void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                     const Properties::PropertyMetadata& metadata,
                                     const std::any& oldValue,
                                     const std::any& newValue,
                                     Properties::PropertyValueSource source) override;
    void OnValueChanged(double value) override;

private:
    void StartDeterminateAnimation() noexcept;
    void StartIndeterminateAnimation() noexcept;
    void StopAnimations() noexcept;
    void UpdateVisualState() noexcept;
    double GetProgressFraction() const noexcept;

    bool m_isIndeterminate = false;
    double m_ringThickness = 6.0;
    float m_animationProgress = 0.0f;
    float m_indeterminatePhase = 0.0f;
    AnimationHandle m_determinateAnimation = kInvalidAnimationHandle;
    AnimationHandle m_indeterminateAnimation = kInvalidAnimationHandle;
    bool m_isInitialized = false;
    std::size_t m_themeAppliedSubscriptionId = 0;
};

}