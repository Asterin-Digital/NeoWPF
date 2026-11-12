#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
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

namespace {

void ApplyScrollBarTheme(ScrollBar& scrollBar)
{
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;
    using NeoWPF::Core::ApplicationThemes::ThemeMetric;
    using NeoWPF::Core::ApplicationThemes::ThemePadding;

    (void)ScrollBar::TrackBrushProperty();
    (void)ScrollBar::ThumbBrushProperty();
    (void)ScrollBar::ThumbSizeProperty();

    auto& themeManager = ThemeManager::Instance();
    scrollBar.SetThemePropertyValue("TrackBrush", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Track));
    scrollBar.SetThemePropertyValue("ThumbBrush", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Thumb));
    scrollBar.SetThemePropertyValue("Background", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Track));
    scrollBar.SetThemePropertyValue("BorderBrush", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Border));

    double thumbSize = 24.0;
    if (themeManager.TryGetMetric(ThemeMetric::ScrollBarThumbSize, thumbSize)) {
        scrollBar.SetThemePropertyValue("ThumbSize", thumbSize);
    }

    double borderThickness = 0.0;
    if (themeManager.TryGetMetric(ThemeMetric::ScrollBarBorderThickness, borderThickness)) {
        scrollBar.SetThemePropertyValue("BorderThickness", borderThickness);
    }

    NeoWPF::UI::Thickness padding;
    if (themeManager.TryGetPadding(ThemePadding::ScrollBar, padding)) {
        scrollBar.SetThemePropertyValue("Padding", padding);
    }
}

}

ScrollBar::ScrollBar()
    : RangeBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ScrollBarTemplate>());
    ApplyScrollBarTheme(*this);
    m_themeAppliedSubscriptionId = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
        ApplyScrollBarTheme(*this);
    });
}

ScrollBar::~ScrollBar()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(m_themeAppliedSubscriptionId);
}

void ScrollBar::SetTrackBrush(std::shared_ptr<Brush> brush)
{
    if (!IsApplyingPropertyValue()) {
        static_cast<Properties::DependencyObject&>(*this).SetValue(TrackBrushProperty(), brush);
        return;
    }
    InvalidateVisual();
}

std::shared_ptr<Brush> ScrollBar::GetTrackBrush() const
{
    return Properties::DependencyObject::GetValue<std::shared_ptr<Brush>>(TrackBrushProperty());
}

void ScrollBar::SetThumbBrush(std::shared_ptr<Brush> brush)
{
    if (!IsApplyingPropertyValue()) {
        static_cast<Properties::DependencyObject&>(*this).SetValue(ThumbBrushProperty(), brush);
        return;
    }
    InvalidateVisual();
}

std::shared_ptr<Brush> ScrollBar::GetThumbBrush() const
{
    return Properties::DependencyObject::GetValue<std::shared_ptr<Brush>>(ThumbBrushProperty());
}

void ScrollBar::SetThumbSize(double size) noexcept
{
    size = size < 1.0 ? 1.0 : size;
    if (!IsApplyingPropertyValue()) {
        static_cast<Properties::DependencyObject&>(*this).SetValue(ThumbSizeProperty(), size);
        return;
    }
    InvalidateMeasure();
    InvalidateVisual();
}

double ScrollBar::GetThumbSize() const noexcept
{
    return Properties::DependencyObject::GetValue<double>(ThumbSizeProperty());
}

void ScrollBar::UpdateScrollRange() noexcept
{
    const double range = (std::max)(0.0, m_contentSize - m_viewportSize);
    RangeBase::SetMinimum(0.0);
    RangeBase::SetMaximum(range);
}

void ScrollBar::SetContentSize(double size) noexcept
{
    m_contentSize = (std::max)(0.0, size);
    UpdateScrollRange();
    InvalidateVisual();
}

void ScrollBar::SetViewportSize(double size) noexcept
{
    m_viewportSize = (std::max)(0.0, size);
    UpdateScrollRange();
    InvalidateVisual();
}

void ScrollBar::SetValue(double value) noexcept
{
    RangeBase::SetValue(value);
}

void ScrollBar::LineUp() noexcept
{
    SetValue(GetValue() - GetSmallChange());
}

void ScrollBar::LineDown() noexcept
{
    SetValue(GetValue() + GetSmallChange());
}

void ScrollBar::PageUp() noexcept
{
    SetValue(GetValue() - GetLargeChange());
}

void ScrollBar::PageDown() noexcept
{
    SetValue(GetValue() + GetLargeChange());
}

double ScrollBar::GetScrollRange() const noexcept
{
    return (std::max)(0.0, m_contentSize - m_viewportSize);
}

double ScrollBar::GetThumbLength(double trackLength) const noexcept
{
    if (trackLength <= 0 || GetScrollRange() <= 0) {
        return std::max(0.0, trackLength);
    }

    const double rawLength = (trackLength * m_viewportSize) / m_contentSize;
    const double thumbLength = std::max(GetThumbSize(), rawLength);
    return std::min(trackLength, std::max(1.0, thumbLength));
}

double ScrollBar::GetThumbPosition(double trackLength, double thumbLength) const noexcept
{
    const double scrollRange = GetScrollRange();
    if (trackLength <= 0 || thumbLength <= 0 || scrollRange <= 0) {
        return 0;
    }

    const double availableTrack = std::max(0.0, trackLength - thumbLength);
    return (GetValue() * availableTrack) / scrollRange;
}

void ScrollBar::UpdateValueFromPosition(double x, double y) noexcept
{
    const bool isVertical = m_orientation == Orientation::Vertical;
    const double trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const double thumbLength = GetThumbLength(trackLength);
    const double maxThumbOffset = std::max(0.0, trackLength - thumbLength);
    if (maxThumbOffset <= 0) {
        SetValue(0);
        return;
    }

    const double pointerPosition = isVertical ? y : x;
    const double clampedPointer = std::clamp(pointerPosition, 0.0, maxThumbOffset);
    const double value = (clampedPointer * GetScrollRange()) / maxThumbOffset;
    SetValue(value);
}

void ScrollBar::Measure(double availableWidth, double availableHeight)
{
    const bool isVertical = m_orientation == Orientation::Vertical;
    const int thickness = 16;
    double primaryAvailable = (std::max)(0.0, isVertical ? availableHeight : availableWidth);

    if (m_viewportSize > 0) {
        primaryAvailable = std::min(primaryAvailable, m_viewportSize);
    }

    const double length = (std::max)(24.0, primaryAvailable);

    if (isVertical) {
        SetDesiredSize(thickness, length);
    } else {
        SetDesiredSize(length, thickness);
    }

}

void ScrollBar::Arrange(double finalWidth, double finalHeight)
{
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

void ScrollBar::OnBorderThicknessChanged(double)
{
    InvalidateVisual();
}

void ScrollBar::OnInitialize()
{
    UIElement::OnInitialize();

    if (auto thumbControl = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        thumbControl->DragDelta += [this](int delta) {
            const bool isVertical = m_orientation == Orientation::Vertical;
            const double trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
            const double thumbLength = GetThumbLength(trackLength);
            const double maxThumbOffset = std::max(0.0, trackLength - thumbLength);
            if (maxThumbOffset <= 0) {
                return;
            }

            const double scrollRange = GetScrollRange();
            if (scrollRange <= 0) {
                return;
            }

            const double valueDelta = delta * scrollRange / maxThumbOffset;
            SetValue(GetValue() + valueDelta);
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

void ScrollBar::UpdateThumbVisualState() noexcept
{
    if (auto thumbControl = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        const bool isVertical = m_orientation == Orientation::Vertical;
        const double trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
        const double trackThickness = isVertical ? GetFinalWidth() : GetFinalHeight();
        const double effectiveTrackLength = std::max(0.0, trackLength);

        const double thumbLength = GetThumbLength(effectiveTrackLength);
        const double thumbPosition = GetThumbPosition(effectiveTrackLength, thumbLength);

        thumbControl->SetVisibility(GetScrollRange() > 0);
        thumbControl->SetBounds(
            static_cast<int>(std::lround(isVertical ? 0.0 : thumbPosition)),
            static_cast<int>(std::lround(isVertical ? thumbPosition : 0.0)),
            static_cast<int>(std::lround(isVertical ? trackThickness : thumbLength)),
            static_cast<int>(std::lround(isVertical ? thumbLength : trackThickness)));
        thumbControl->SetOrientation(m_orientation);
        thumbControl->SetThemePropertyValue("Background", GetThumbBrush());
        thumbControl->SetThemePropertyValue("BorderBrush",
            NeoWPF::Core::ApplicationThemes::GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Border));
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
    double absX = 0.0;
    double absY = 0.0;
    renderTransform.positionMatrix.TransformPoint(0.0f, 0.0f, absX, absY);
    const auto clip = GetClipRect();

    auto track = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Track"));
    auto thumb = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Thumb"));

    if (GetScrollRange() <= 0) {
        return;
    }

    const bool isVertical = m_orientation == Orientation::Vertical;
    const double trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const double trackThickness = isVertical ? GetFinalWidth() : GetFinalHeight();
    const double effectiveTrackLength = (std::max)(0.0, trackLength);

    if (effectiveTrackLength <= 0 || trackThickness <= 0) {
        return;
    }

    const double thumbLength = GetThumbLength(effectiveTrackLength);
    const double thumbPosition = GetThumbPosition(effectiveTrackLength, thumbLength);

    const auto cornerRadius = GetCornerRadius();

    UpdateThumbVisualState();

    if (track) {
        if (isVertical) {
            track->SetRect(absX, absY, trackThickness, trackLength);
        } else {
            track->SetRect(absX, absY, trackLength, trackThickness);
        }
        track->SetFillBrush(GetTrackBrush());
        track->SetBorderThickness(0);
        track->SetCornerRadius(cornerRadius);
        track->SetOpacity(GetOpacity());
        track->SetClipRect(clip);
        track->BuildRenderCommands(commands);
    } else if (isVertical) {
        const auto trackBrush = std::dynamic_pointer_cast<SolidColorBrush>(GetTrackBrush());
        commands.DrawRoundedRect(absX, absY, trackThickness, trackLength, cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, NeoWPF::Rendering::Paint::Solid(trackBrush ? trackBrush->GetArgb() : 0u), GetOpacity(), clip);
    } else {
        const auto trackBrush = std::dynamic_pointer_cast<SolidColorBrush>(GetTrackBrush());
        commands.DrawRoundedRect(absX, absY, trackLength, trackThickness, cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, NeoWPF::Rendering::Paint::Solid(trackBrush ? trackBrush->GetArgb() : 0u), GetOpacity(), clip);
    }

    if (auto thumbControl = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        thumbControl->InvalidateVisual();
        thumbControl->BuildRenderCommands(commands);
    } else if (thumb) {
        if (isVertical) {
            thumb->SetRect(absX, absY + thumbPosition, trackThickness, thumbLength);
        } else {
            thumb->SetRect(absX + thumbPosition, absY, thumbLength, trackThickness);
        }
        thumb->SetFillBrush(GetThumbBrush());
        thumb->SetBorderThickness(0);
        thumb->SetCornerRadius(cornerRadius);
        thumb->SetOpacity(GetOpacity());
        thumb->SetClipRect(clip);
        thumb->BuildRenderCommands(commands);
    }

}

}
