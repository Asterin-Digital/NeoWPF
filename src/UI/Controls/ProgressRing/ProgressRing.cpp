#include <NeoWPF/UI/Controls/ProgressRing/ProgressRing.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Animation/Definitions/DoubleAnimation.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

#include <algorithm>
#include <cmath>
#include <memory>

namespace NeoWPF::UI {
namespace {

constexpr double kDefaultSize = 48.0;
constexpr float kDeterminateDuration = 0.18f;
constexpr float kIndeterminateDuration = 1.2f;
constexpr double kIndeterminateSegmentRatio = 0.28;
constexpr double kTwoPi = 6.28318530717958647692;

void ApplyProgressRingTheme(ProgressRing& progressRing)
{
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;
    progressRing.SetThemePropertyValue(
        "Background",
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Track));
}

}

ProgressRing::ProgressRing()
{
    (void)MinimumProperty();
    (void)MaximumProperty();
    (void)ValueProperty();
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    ApplyProgressRingTheme(*this);
    m_themeAppliedSubscriptionId =
        NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
            ApplyProgressRingTheme(*this);
            InvalidateVisual();
        });
    m_animationProgress = static_cast<float>(GetProgressFraction());
}

ProgressRing::~ProgressRing()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(
        m_themeAppliedSubscriptionId);
    StopAnimations();
}

void ProgressRing::SetIsIndeterminate(bool value) noexcept
{
    if (!IsApplyingPropertyValue()) {
        DependencyObject::SetValue(IsIndeterminateProperty(), value);
        return;
    }
    if (m_isIndeterminate == value) {
        return;
    }

    m_isIndeterminate = value;
    StopAnimations();
    UpdateVisualState();
}

void ProgressRing::SetRingThickness(double thickness) noexcept
{
    if (!IsApplyingPropertyValue()) {
        DependencyObject::SetValue(RingThicknessProperty(), thickness);
        return;
    }
    const double clampedThickness = std::max(0.0, thickness);
    if (m_ringThickness == clampedThickness) {
        return;
    }

    m_ringThickness = clampedThickness;
    InvalidateVisual();
}

void ProgressRing::SetAnimationProgress(float value) noexcept
{
    m_animationProgress = std::clamp(value, 0.0f, 1.0f);
    InvalidateVisual();
}

void ProgressRing::SetIndeterminatePhase(float value) noexcept
{
    m_indeterminatePhase = value - std::floor(value);
    if (m_indeterminatePhase < 0.0f) {
        m_indeterminatePhase += 1.0f;
    }
    InvalidateVisual();
}

void ProgressRing::Measure(double availableWidth, double availableHeight)
{
    const auto desiredDimension = [](double requested, double available) {
        double desired = requested > 0.0 ? requested : kDefaultSize;
        if (available > 0.0) {
            desired = std::min(desired, available);
        }
        return desired;
    };

    SetDesiredSize(
        desiredDimension(GetWidth(), availableWidth),
        desiredDimension(GetHeight(), availableHeight));
}

void ProgressRing::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    UpdateVisualState();
}

void ProgressRing::OnInitialize()
{
    UIElement::OnInitialize();
    m_isInitialized = true;
    UpdateVisualState();
}

void ProgressRing::OnDependencyPropertyChanged(
    const Properties::DependencyProperty& property,
    const Properties::PropertyMetadata& metadata,
    const std::any& oldValue,
    const std::any& newValue,
    Properties::PropertyValueSource source)
{
    RangeBase::OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);
    if (property.GetName() == "Minimum" || property.GetName() == "Maximum") {
        StartDeterminateAnimation();
    }
}

void ProgressRing::OnValueChanged(double value)
{
    StartDeterminateAnimation();
    RangeBase::OnValueChanged(value);
}

void ProgressRing::StartDeterminateAnimation() noexcept
{
    if (m_isIndeterminate) {
        return;
    }

    const float target = static_cast<float>(GetProgressFraction());
    if (!m_isInitialized) {
        m_animationProgress = target;
        InvalidateVisual();
        return;
    }

    if (m_indeterminateAnimation != kInvalidAnimationHandle) {
        StopStoryboard(m_indeterminateAnimation);
        m_indeterminateAnimation = kInvalidAnimationHandle;
        m_indeterminatePhase = 0.0f;
    }

    if (std::abs(m_animationProgress - target) < 0.0001f) {
        m_animationProgress = target;
        InvalidateVisual();
        return;
    }

    if (m_determinateAnimation != kInvalidAnimationHandle) {
        StopStoryboard(m_determinateAnimation);
        m_determinateAnimation = kInvalidAnimationHandle;
    }

    auto animation = std::make_shared<Animation::DoubleAnimation>();
    animation->SetFrom(m_animationProgress);
    animation->SetTo(target);
    animation->SetDuration(kDeterminateDuration);

    auto storyboard = std::make_shared<Animation::Storyboard>();
    storyboard->AddChild({}, AnimationProgressProperty().GetName(), animation);
    m_determinateAnimation = BeginStoryboard(storyboard, this);
    if (m_determinateAnimation == kInvalidAnimationHandle) {
        m_animationProgress = target;
        InvalidateVisual();
    }
}

void ProgressRing::StartIndeterminateAnimation() noexcept
{
    if (!m_isIndeterminate || !m_isInitialized ||
        m_indeterminateAnimation != kInvalidAnimationHandle) {
        return;
    }

    if (m_determinateAnimation != kInvalidAnimationHandle) {
        StopStoryboard(m_determinateAnimation);
        m_determinateAnimation = kInvalidAnimationHandle;
    }

    m_indeterminatePhase = 0.0f;
    auto animation = std::make_shared<Animation::DoubleAnimation>();
    animation->SetFrom(0.0f);
    animation->SetTo(1.0f);
    animation->SetDuration(kIndeterminateDuration);
    animation->SetRepeatBehavior(Animation::RepeatBehavior::Loop);

    auto storyboard = std::make_shared<Animation::Storyboard>();
    storyboard->AddChild({}, IndeterminatePhaseProperty().GetName(), animation);
    m_indeterminateAnimation = BeginStoryboard(storyboard, this);
}

void ProgressRing::StopAnimations() noexcept
{
    if (m_determinateAnimation != kInvalidAnimationHandle) {
        StopStoryboard(m_determinateAnimation);
        m_determinateAnimation = kInvalidAnimationHandle;
    }
    if (m_indeterminateAnimation != kInvalidAnimationHandle) {
        StopStoryboard(m_indeterminateAnimation);
        m_indeterminateAnimation = kInvalidAnimationHandle;
    }
}

void ProgressRing::UpdateVisualState() noexcept
{
    if (m_isInitialized) {
        if (m_isIndeterminate) {
            StartIndeterminateAnimation();
        } else {
            StartDeterminateAnimation();
        }
    }
    InvalidateVisual();
}

double ProgressRing::GetProgressFraction() const noexcept
{
    const double range = GetMaximum() - GetMinimum();
    if (range <= 0.0) {
        return GetValue() >= GetMaximum() ? 1.0 : 0.0;
    }
    return std::clamp((GetValue() - GetMinimum()) / range, 0.0, 1.0);
}

void ProgressRing::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const double width = GetFinalWidth();
    const double height = GetFinalHeight();
    const double size = std::min(width, height);
    if (size <= 0.0) {
        return;
    }

    const double thickness = std::min(m_ringThickness, size * 0.5);
    if (thickness <= 0.0) {
        return;
    }

    const auto transform = GetWorldRenderTransform();
    double absoluteX = 0.0;
    double absoluteY = 0.0;
    transform.positionMatrix.TransformPoint(0.0f, 0.0f, absoluteX, absoluteY);
    const double radius = (size - thickness) * 0.5;
    const double centerX = absoluteX + width * 0.5;
    const double centerY = absoluteY + height * 0.5;
    const auto clip = GetClipRect();

    const auto track = std::dynamic_pointer_cast<SolidColorBrush>(GetBackgroundBrush());
    if (track) {
        commands.DrawArc(centerX, centerY, radius, thickness, 0.0, kTwoPi,
            NeoWPF::Rendering::Paint::Solid(track->GetArgb()), static_cast<float>(GetOpacity() * track->GetOpacity()), clip);
    }

    const auto accent = std::dynamic_pointer_cast<SolidColorBrush>(
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Accent));
    if (!accent) {
        return;
    }

    const double startAngle = m_isIndeterminate
        ? kTwoPi * static_cast<double>(m_indeterminatePhase)
        : 0.0;
    const double sweepAngle = m_isIndeterminate
        ? kTwoPi * kIndeterminateSegmentRatio
        : kTwoPi * static_cast<double>(m_animationProgress);
    if (sweepAngle <= 0.0) {
        return;
    }

    commands.DrawArc(centerX, centerY, radius, thickness, startAngle, sweepAngle,
        NeoWPF::Rendering::Paint::Solid(accent->GetArgb()), static_cast<float>(GetOpacity() * accent->GetOpacity()), clip);
}

}