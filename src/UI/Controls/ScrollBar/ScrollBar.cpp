#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <cmath>
#include <memory>
#include <algorithm>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

ScrollBar::ScrollBar()
    : RangeBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ScrollBarTemplate>());

    SetPadding(0);
    SetBackgroundColor(NeoWPF::UI::Color::FromArgb(0xFFEAEAEAu));
    SetBorderColor(NeoWPF::UI::Color::FromArgb(0xFF707070u));
    SetBorderThickness(0);
}

ScrollBar::~ScrollBar() = default;

void ScrollBar::UpdateScrollRange() noexcept
{
    const int range = std::max(0, m_contentSize - m_viewportSize);
    RangeBase::SetMinimum(0.0);
    RangeBase::SetMaximum(static_cast<double>(range));
}

void ScrollBar::SetContentSize(int size) noexcept
{
    m_contentSize = std::max(0, size);
    UpdateScrollRange();
    InvalidateVisual();
}

void ScrollBar::SetViewportSize(int size) noexcept
{
    m_viewportSize = std::max(0, size);
    UpdateScrollRange();
    InvalidateVisual();
}

void ScrollBar::SetValue(int value) noexcept
{
    RangeBase::SetValue(static_cast<double>(value));
}

void ScrollBar::LineUp() noexcept
{
    SetValue(static_cast<int>(GetValue() - GetSmallChange()));
}

void ScrollBar::LineDown() noexcept
{
    SetValue(static_cast<int>(GetValue() + GetSmallChange()));
}

void ScrollBar::PageUp() noexcept
{
    SetValue(static_cast<int>(GetValue() - GetLargeChange()));
}

void ScrollBar::PageDown() noexcept
{
    SetValue(static_cast<int>(GetValue() + GetLargeChange()));
}

int ScrollBar::GetScrollRange() const noexcept
{
    return std::max(0, m_contentSize - m_viewportSize);
}

int ScrollBar::GetThumbLength(int trackLength) const noexcept
{
    if (trackLength <= 0 || GetScrollRange() <= 0) {
        return std::max(0, trackLength);
    }

    const int rawLength = (trackLength * m_viewportSize) / m_contentSize;
    const int thumbLength = std::max(m_thumbSize, rawLength);
    return std::min(trackLength, std::max(1, thumbLength));
}

int ScrollBar::GetThumbPosition(int trackLength, int thumbLength) const noexcept
{
    const int scrollRange = GetScrollRange();
    if (trackLength <= 0 || thumbLength <= 0 || scrollRange <= 0) {
        return 0;
    }

    const int availableTrack = std::max(0, trackLength - thumbLength);
    return static_cast<int>((GetValue() * static_cast<double>(availableTrack)) / scrollRange);
}

void ScrollBar::UpdateValueFromPosition(int x, int y) noexcept
{
    const bool isVertical = m_orientation == Orientation::Vertical;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int thumbLength = GetThumbLength(trackLength);
    const int maxThumbOffset = std::max(0, trackLength - thumbLength);
    if (maxThumbOffset <= 0) {
        SetValue(0);
        return;
    }

    const int pointerPosition = isVertical ? y : x;
    const int clampedPointer = std::clamp(pointerPosition, 0, maxThumbOffset);
    const int value = static_cast<int>((clampedPointer * static_cast<double>(GetScrollRange())) / maxThumbOffset);
    SetValue(value);
}

void ScrollBar::Measure(int availableWidth, int availableHeight)
{
    const bool isVertical = m_orientation == Orientation::Vertical;
    const int thickness = 16;
    const int minLength = 24;
    int primaryAvailable = std::max(0, isVertical ? availableHeight : availableWidth);

    if (m_viewportSize > 0) {
        primaryAvailable = std::min(primaryAvailable, m_viewportSize);
    }

    const int length = std::max(minLength, primaryAvailable);

    if (isVertical) {
        SetDesiredSize(thickness, length);
    } else {
        SetDesiredSize(length, thickness);
    }

}

void ScrollBar::Arrange(int finalWidth, int finalHeight)
{
    const bool isVertical = m_orientation == Orientation::Vertical;
    int effectiveWidth = finalWidth;
    int effectiveHeight = finalHeight;

    FrameworkElement::Arrange(finalWidth, finalHeight);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug, "[ScrollBar] Arrange: final=%dx%d orientation=%d value=%d content=%d viewport=%d", finalWidth, finalHeight, static_cast<int>(m_orientation), GetValue(), m_contentSize, m_viewportSize);
    UpdateScrollState();
    UpdateThumbVisualState();
}

void ScrollBar::SetOrientation(Orientation orientation) noexcept
{
    static_cast<NeoWPF::UI::Properties::DependencyObject&>(*this).SetValue(GetOrientationProperty(), static_cast<int>(orientation));
}

void ScrollBar::OnValueChanged(double value)
{
    InvalidateVisual();
    UpdateThumbVisualState();
    ValueChanged.Invoke(static_cast<int>(value));
}

void ScrollBar::OnPaddingChanged(Thickness padding)
{
    (void)padding;
    InvalidateVisual();
}

void ScrollBar::OnBackgroundColorChanged(NeoWPF::UI::Color)
{
    InvalidateVisual();
}

void ScrollBar::OnBorderColorChanged(NeoWPF::UI::Color)
{
    InvalidateVisual();
}

void ScrollBar::OnBorderThicknessChanged(int)
{
    InvalidateVisual();
}

void ScrollBar::OnInitialize()
{
    UIElement::OnInitialize();

    if (auto thumbControl = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        thumbControl->DragDelta += [this](int delta) {
            const bool isVertical = m_orientation == Orientation::Vertical;
            const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
            const int thumbLength = GetThumbLength(trackLength);
            const int maxThumbOffset = std::max(0, trackLength - thumbLength);
            if (maxThumbOffset <= 0) {
                return;
            }

            const int scrollRange = GetScrollRange();
            if (scrollRange <= 0) {
                return;
            }

            const double valueDelta = delta * static_cast<double>(scrollRange) / maxThumbOffset;
            SetValue(static_cast<int>(std::lround(GetValue() + valueDelta)));
        };
    }
}

int ScrollBar::GetOrientationValue() const noexcept
{
    try {
        return static_cast<const NeoWPF::UI::Properties::DependencyObject*>(this)->GetValue<int>(GetOrientationProperty());
    } catch (const std::bad_any_cast&) {
        return static_cast<int>(m_orientation);
    }
}

void ScrollBar::SetOrientationValue(int orientation) noexcept
{
    const auto nextOrientation = static_cast<Orientation>(orientation);
    if (m_orientation == nextOrientation) {
        return;
    }

    m_orientation = nextOrientation;
    if (auto thumbControl = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        thumbControl->SetOrientation(m_orientation);
    }
    InvalidateVisual();
}

const NeoWPF::UI::Properties::DependencyProperty& ScrollBar::GetOrientationProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<ScrollBar, int>(
        "Orientation",
        &ScrollBar::GetOrientationValue,
        &ScrollBar::SetOrientationValue);
    return property;
}

void ScrollBar::UpdateThumbVisualState() noexcept
{
    if (auto thumbControl = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        const bool isVertical = m_orientation == Orientation::Vertical;
        const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
        const int trackThickness = isVertical ? GetFinalWidth() : GetFinalHeight();
        const int effectiveTrackLength = std::max(0, trackLength);

        const int thumbLength = GetThumbLength(effectiveTrackLength);
        const int thumbPosition = GetThumbPosition(effectiveTrackLength, thumbLength);

        thumbControl->SetVisible(GetScrollRange() > 0);
        thumbControl->SetBounds(
            isVertical ? 0 : thumbPosition,
            isVertical ? thumbPosition : 0,
            isVertical ? trackThickness : thumbLength,
            isVertical ? thumbLength : trackThickness);
        thumbControl->SetOrientation(m_orientation);
        thumbControl->SetBackgroundColor(m_thumbColor);
        thumbControl->SetBorderColor(NeoWPF::UI::Color::FromArgb(0xFF707070u));
        thumbControl->SetBorderThickness(0);
        thumbControl->SetOpacity(GetOpacity());
    }
}

void ScrollBar::UpdateScrollState() noexcept
{
    UpdateScrollRange();
    UpdateThumbVisualState();
}

void ScrollBar::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const auto renderTransform = GetWorldRenderTransform();
    float absX = 0.0f;
    float absY = 0.0f;
    renderTransform.positionMatrix.TransformPoint(0.0f, 0.0f, absX, absY);
    const auto clip = GetClipRect();

    auto track = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Track"));
    auto thumb = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Thumb"));

    if (GetScrollRange() <= 0) {
        return;
    }

    const bool isVertical = m_orientation == Orientation::Vertical;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int trackThickness = isVertical ? GetFinalWidth() : GetFinalHeight();
    const int effectiveTrackLength = std::max(0, trackLength);

    if (effectiveTrackLength <= 0 || trackThickness <= 0) {
        return;
    }

    const int thumbLength = GetThumbLength(effectiveTrackLength);
    const int thumbPosition = GetThumbPosition(effectiveTrackLength, thumbLength);

    const auto cornerRadius = GetCornerRadius();

    UpdateThumbVisualState();

    if (auto thumbControl = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        thumbControl->InvalidateVisual();
        thumbControl->BuildRenderCommands(commands);
    }

    if (track) {
        if (isVertical) {
            track->SetRect(static_cast<int>(std::lround(absX)), static_cast<int>(std::lround(absY)), trackThickness, trackLength);
        } else {
            track->SetRect(static_cast<int>(std::lround(absX)), static_cast<int>(std::lround(absY)), trackLength, trackThickness);
        }
        track->SetFillColor(m_trackColor.ToArgb());
        track->SetBorderColor(0x00000000u);
        track->SetBorderThickness(0);
        track->SetCornerRadius(cornerRadius);
        track->SetOpacity(GetOpacity());
        track->SetClipRect(clip);
        track->BuildRenderCommands(commands);
    } else if (isVertical) {
        commands.DrawRoundedRect(static_cast<float>(absX), static_cast<float>(absY), static_cast<float>(trackThickness), static_cast<float>(trackLength), cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, m_trackColor.ToArgb(), GetOpacity(), clip);
    } else {
        commands.DrawRoundedRect(static_cast<float>(absX), static_cast<float>(absY), static_cast<float>(trackLength), static_cast<float>(trackThickness), cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, m_trackColor.ToArgb(), GetOpacity(), clip);
    }

    if (thumb) {
        if (isVertical) {
            thumb->SetRect(static_cast<int>(std::lround(absX)), static_cast<int>(std::lround(absY)) + thumbPosition, trackThickness, thumbLength);
        } else {
            thumb->SetRect(static_cast<int>(std::lround(absX)) + thumbPosition, static_cast<int>(std::lround(absY)), thumbLength, trackThickness);
        }
        thumb->SetFillColor(m_thumbColor.ToArgb());
        thumb->SetBorderColor(0x00000000u);
        thumb->SetBorderThickness(0);
        thumb->SetCornerRadius(cornerRadius);
        thumb->SetOpacity(GetOpacity());
        thumb->SetClipRect(clip);
        thumb->BuildRenderCommands(commands);
    } else if (isVertical) {
        commands.DrawRoundedRect(static_cast<float>(absX), static_cast<float>(absY + thumbPosition), static_cast<float>(trackThickness), static_cast<float>(thumbLength), cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, m_thumbColor.ToArgb(), GetOpacity(), clip);
    } else {
        commands.DrawRoundedRect(static_cast<float>(absX + thumbPosition), static_cast<float>(absY), static_cast<float>(thumbLength), static_cast<float>(trackThickness), cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, m_thumbColor.ToArgb(), GetOpacity(), clip);
    }

}

}
