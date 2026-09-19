#include <NeoWPF/UI/Controls/ProgressBar/ProgressBar.h>
#include <NeoWPF/UI/Animation/Definitions/DoubleAnimation.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <algorithm>
#include <cmath>
#include <memory>

namespace NeoWPF::UI {

namespace {

constexpr double kDefaultThickness = 16.0;
constexpr double kMinimumLength = 24.0;
constexpr float kDeterminateDuration = 0.18f;
constexpr float kIndeterminateDuration = 1.2f;
constexpr double kIndeterminateSegmentRatio = 0.28;
constexpr double kMinimumIndeterminateSegment = 12.0;

}

ProgressBar::ProgressBar()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ProgressBarTemplate>());
    SetPadding(0);
    SetBackgroundColor(Color::FromArgb(0xFFEAEAEAu));
    SetBorderColor(Color::FromArgb(0xFF707070u));
    SetBorderThickness(0);
    m_animationProgress = static_cast<float>(GetProgressFraction());
}

ProgressBar::~ProgressBar()
{
    StopAnimations();
}

void ProgressBar::SetOrientation(Orientation orientation) noexcept
{
    static_cast<Properties::DependencyObject&>(*this).SetValue(
        GetOrientationProperty(), static_cast<int>(orientation));
}

void ProgressBar::SetIsIndeterminate(bool value) noexcept
{
    if (m_isIndeterminate == value) {
        return;
    }
    m_isIndeterminate = value;
    StopAnimations();
    UpdateVisualState();
}

void ProgressBar::SetOrientationValue(int orientation) noexcept
{
    m_orientation = static_cast<Orientation>(orientation);
    InvalidateMeasure();
    InvalidateVisual();
}

int ProgressBar::GetOrientationValue() const noexcept
{
    return static_cast<int>(m_orientation);
}

void ProgressBar::SetAnimationProgress(float value) noexcept
{
    m_animationProgress = std::clamp(value, 0.0f, 1.0f);
    InvalidateVisual();
}

void ProgressBar::SetIndeterminatePhase(float value) noexcept
{
    m_indeterminatePhase = value - std::floor(value);
    if (m_indeterminatePhase < 0.0f) {
        m_indeterminatePhase += 1.0f;
    }
    InvalidateVisual();
}

void ProgressBar::Measure(double availableWidth, double availableHeight)
{
    const bool isVertical = m_orientation == Orientation::Vertical;
    const double primaryAvailable = std::max(0.0, isVertical ? availableHeight : availableWidth);
    const double length = std::max(kMinimumLength, primaryAvailable);

    if (isVertical) {
        SetDesiredSize(kDefaultThickness, length);
    } else {
        SetDesiredSize(length, kDefaultThickness);
    }
}

void ProgressBar::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    UpdateVisualState();
}

void ProgressBar::OnInitialize()
{
    UIElement::OnInitialize();
    m_isInitialized = true;
    UpdateVisualState();
}

void ProgressBar::OnValueChanged(double)
{
    StartDeterminateAnimation();
    RangeBase::OnValueChanged(GetValue());
}

void ProgressBar::StartDeterminateAnimation() noexcept
{
    if (m_isIndeterminate) {
        return;
    }

    if (m_indeterminateAnimation != kInvalidAnimationHandle) {
        StopStoryboard(m_indeterminateAnimation);
        m_indeterminateAnimation = kInvalidAnimationHandle;
        m_indeterminatePhase = 0.0f;
    }

    const float target = static_cast<float>(GetProgressFraction());
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

void ProgressBar::StartIndeterminateAnimation() noexcept
{
    if (!m_isIndeterminate) {
        return;
    }

    if (m_determinateAnimation != kInvalidAnimationHandle) {
        StopStoryboard(m_determinateAnimation);
        m_determinateAnimation = kInvalidAnimationHandle;
    }

    if (m_indeterminateAnimation != kInvalidAnimationHandle) {
        return;
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

void ProgressBar::StopAnimations() noexcept
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

void ProgressBar::UpdateVisualState() noexcept
{
    if (m_isIndeterminate) {
        StartIndeterminateAnimation();
    } else {
        StartDeterminateAnimation();
    }
    InvalidateVisual();
}

double ProgressBar::GetProgressFraction() const noexcept
{
    const double range = GetMaximum() - GetMinimum();
    if (range <= 0.0) {
        return GetValue() >= GetMaximum() ? 1.0 : 0.0;
    }
    return std::clamp((GetValue() - GetMinimum()) / range, 0.0, 1.0);
}

void ProgressBar::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const auto transform = GetWorldRenderTransform();
    double absX = 0.0;
    double absY = 0.0;
    transform.positionMatrix.TransformPoint(0.0f, 0.0f, absX, absY);
    const auto clip = GetClipRect();
    const bool isVertical = m_orientation == Orientation::Vertical;
    const double width = GetFinalWidth();
    const double height = GetFinalHeight();
    const double primaryLength = isVertical ? height : width;
    const double crossLength = isVertical ? width : height;

    auto track = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Track"));
    auto indicator = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Indicator"));
    if (!track || !indicator || primaryLength <= 0.0 || crossLength <= 0.0) {
        return;
    }

    track->SetRect(absX, absY, width, height);
    track->SetClipRect(clip);
    track->BuildRenderCommands(commands);

    double start = 0.0;
    double length = primaryLength * static_cast<double>(m_animationProgress);
    if (m_isIndeterminate) {
        length = std::max(kMinimumIndeterminateSegment, primaryLength * kIndeterminateSegmentRatio);
        start = (primaryLength + length) * static_cast<double>(m_indeterminatePhase) - length;
    }

    const double visibleStart = std::clamp(start, 0.0, primaryLength);
    const double visibleEnd = std::clamp(start + length, 0.0, primaryLength);
    length = std::max(0.0, visibleEnd - visibleStart);
    if (length <= 0.0) {
        return;
    }

    if (isVertical) {
        indicator->SetRect(absX, absY + primaryLength - visibleEnd, crossLength, length);
    } else {
        indicator->SetRect(absX + visibleStart, absY, length, crossLength);
    }
    indicator->SetClipRect(clip);
    indicator->BuildRenderCommands(commands);
}

}
