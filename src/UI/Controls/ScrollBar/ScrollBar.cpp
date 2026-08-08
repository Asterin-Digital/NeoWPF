#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
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
    SetBackgroundColor(NeoWPF::UI::Color::FromArgb(0x00000000u));
    SetBorderColor(NeoWPF::UI::Color::FromArgb(0x00000000u));
    SetBorderThickness(0);

    SetClipToBounds(true);

    m_thumb = std::make_shared<Thumb>();
    m_thumb->SetOrientation(m_orientation);
}

ScrollBar::~ScrollBar() = default;

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

    if (isVertical) {
        const int viewportHeight = m_viewportElement ? std::max(0, m_viewportElement->GetDesiredHeight()) : 0;
        if (viewportHeight > 0) {
            effectiveHeight = std::min(finalHeight, viewportHeight);
        }
    } else {
        const int viewportWidth = m_viewportElement ? std::max(0, m_viewportElement->GetDesiredWidth()) : 0;
        if (viewportWidth > 0) {
            effectiveWidth = std::min(finalWidth, viewportWidth);
        }
    }

    FrameworkElement::Arrange(effectiveWidth, effectiveHeight);
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
    UpdateScrollTarget();
    ValueChanged.Invoke(static_cast<int>(value));
}

void ScrollBar::OnPaddingChanged(int padding)
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

void ScrollBar::UpdateScrollTarget() noexcept
{
    if (!m_scrollTarget) {
        return;
    }

    const int value = GetValue();
    if (m_orientation == Orientation::Vertical) {
        m_scrollTarget->SetRenderOffset(0, -value);
    } else {
        m_scrollTarget->SetRenderOffset(-value, 0);
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
    if (m_thumb) {
        m_thumb->SetOrientation(m_orientation);
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

        int absX = 0;
        int absY = 0;
        GetAbsolutePosition(absX, absY);

        thumbControl->SetVisible(GetScrollRange() > 0);
        thumbControl->SetBounds(
            isVertical ? absX : absX + thumbPosition,
            isVertical ? absY + thumbPosition : absY,
            isVertical ? trackThickness : thumbLength,
            isVertical ? thumbLength : trackThickness);
        thumbControl->SetOrientation(m_orientation);
        thumbControl->SetBackgroundColor(m_thumbColor);
        thumbControl->SetBorderColor(NeoWPF::UI::Color::FromArgb(0x00000000u));
        thumbControl->SetBorderThickness(0);
        thumbControl->SetOpacity(GetOpacity());
    }
}

void ScrollBar::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX, absY;
    GetAbsolutePosition(absX, absY);
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
        thumbControl->BuildRenderCommands(commands);
    }

    if (track) {
        if (isVertical) {
            track->SetRect(absX, absY, trackThickness, trackLength);
        } else {
            track->SetRect(absX, absY, trackLength, trackThickness);
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
            thumb->SetRect(absX, absY + thumbPosition, trackThickness, thumbLength);
        } else {
            thumb->SetRect(absX + thumbPosition, absY, thumbLength, trackThickness);
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
