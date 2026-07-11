#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
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
    : Content::ContentControl()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ScrollBarTemplate>());
    ApplyTemplate();

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(0);
    SetPresenter(presenter);

    SetWidth(16);
    SetHeight(120);
    SetClipToBounds(true);
}

ScrollBar::~ScrollBar() = default;

void ScrollBar::Measure(int availableWidth, int availableHeight)
{
    const bool isVertical = m_orientation == ScrollBarOrientation::Vertical;
    const int thickness = 16;
    const int minLength = 24;
    const int primaryAvailable = isVertical ? availableHeight : availableWidth;
    const int length = std::max(minLength, std::max(0, primaryAvailable));

    if (isVertical) {
        SetDesiredSize(thickness, length);
    } else {
        SetDesiredSize(length, thickness);
    }

    if (auto presenter = GetPresenter()) {
        presenter->Measure(availableWidth, availableHeight);
    }
}

void ScrollBar::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    if (auto presenter = GetPresenter()) {
        presenter->Arrange(finalWidth, finalHeight);
    }
    if (s_scrollBarDebug) {
        NeoWPF::Diagnostics::LogInfo("[ScrollBar] Arrange: final=%dx%d orientation=%d value=%d content=%d viewport=%d", finalWidth, finalHeight, static_cast<int>(m_orientation), m_value, m_contentSize, m_viewportSize);
    }
    UpdateScrollState();
}

void ScrollBar::UpdateScrollTarget() noexcept
{
    if (!m_scrollTarget) {
        return;
    }

    if (m_orientation == ScrollBarOrientation::Vertical) {
        m_scrollTarget->SetRenderOffset(0, -m_value);
    } else {
        m_scrollTarget->SetRenderOffset(-m_value, 0);
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

    const bool isVertical = m_orientation == ScrollBarOrientation::Vertical;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int trackThickness = isVertical ? GetFinalWidth() : GetFinalHeight();

    if (trackLength <= 0 || trackThickness <= 0) {
        return;
    }

    const int thumbLength = GetThumbLength(trackLength);
    const int thumbPosition = GetThumbPosition(trackLength, thumbLength);

    const auto cornerRadius = GetCornerRadius();

    if (track) {
        if (isVertical) {
            track->SetRect(absX, absY, trackThickness, trackLength);
        } else {
            track->SetRect(absX, absY, trackLength, trackThickness);
        }
        track->SetFillColor(m_trackColor);
        track->SetBorderColor(0x00000000u);
        track->SetBorderThickness(0);
        track->SetCornerRadius(cornerRadius);
        track->SetOpacity(GetOpacity());
        track->SetClipRect(clip);
        track->BuildRenderCommands(commands);
    } else if (isVertical) {
        commands.DrawRoundedRect(static_cast<float>(absX), static_cast<float>(absY), static_cast<float>(trackThickness), static_cast<float>(trackLength), cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, m_trackColor, GetOpacity(), clip);
    } else {
        commands.DrawRoundedRect(static_cast<float>(absX), static_cast<float>(absY), static_cast<float>(trackLength), static_cast<float>(trackThickness), cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, m_trackColor, GetOpacity(), clip);
    }

    if (thumb) {
        if (isVertical) {
            thumb->SetRect(absX, absY + thumbPosition, trackThickness, thumbLength);
        } else {
            thumb->SetRect(absX + thumbPosition, absY, thumbLength, trackThickness);
        }
        thumb->SetFillColor(m_thumbColor);
        thumb->SetBorderColor(0x00000000u);
        thumb->SetBorderThickness(0);
        thumb->SetCornerRadius(cornerRadius);
        thumb->SetOpacity(GetOpacity());
        thumb->SetClipRect(clip);
        thumb->BuildRenderCommands(commands);
    } else if (isVertical) {
        commands.DrawRoundedRect(static_cast<float>(absX), static_cast<float>(absY + thumbPosition), static_cast<float>(trackThickness), static_cast<float>(thumbLength), cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, m_thumbColor, GetOpacity(), clip);
    } else {
        commands.DrawRoundedRect(static_cast<float>(absX + thumbPosition), static_cast<float>(absY), static_cast<float>(thumbLength), static_cast<float>(trackThickness), cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft, m_thumbColor, GetOpacity(), clip);
    }

    if (auto presenter = GetPresenter()) {
        presenter->BuildRenderCommands(commands);
    }
}

}
