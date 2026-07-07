#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <memory>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

ScrollBar::ScrollBar()
    : FrameworkElement()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ScrollBarTemplate>());
    ApplyTemplate();
    SetWidth(16);
    SetHeight(120);
    SetClipToBounds(true);
}

ScrollBar::~ScrollBar() = default;

void ScrollBar::Measure(int availableWidth, int availableHeight)
{
    const bool isVertical = m_orientation == ScrollBarOrientation::Vertical;
    SetDesiredSize(isVertical ? 16 : 120, isVertical ? 120 : 16);
}

void ScrollBar::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
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

    if (track) {
        track->SetRect(absX, absY, GetFinalWidth(), GetFinalHeight());
        track->SetFillColor(m_trackColor);
        track->SetOpacity(GetOpacity());
        track->SetClipRect(clip);
        track->BuildRenderCommands(commands);
    } else {
        commands.DrawRect(static_cast<float>(absX), static_cast<float>(absY), static_cast<float>(GetFinalWidth()), static_cast<float>(GetFinalHeight()), m_trackColor, GetOpacity(), clip);
    }

    const bool isVertical = m_orientation == ScrollBarOrientation::Vertical;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int thumbLength = GetThumbLength(trackLength);
    const int thumbPosition = GetThumbPosition(trackLength, thumbLength);

    if (thumb) {
        if (isVertical) {
            thumb->SetRect(absX, absY + thumbPosition, GetFinalWidth(), thumbLength);
        } else {
            thumb->SetRect(absX + thumbPosition, absY, thumbLength, GetFinalHeight());
        }
        thumb->SetFillColor(m_thumbColor);
        thumb->SetOpacity(GetOpacity());
        thumb->SetClipRect(clip);
        thumb->BuildRenderCommands(commands);
    } else if (isVertical) {
        commands.DrawRect(static_cast<float>(absX), static_cast<float>(absY + thumbPosition), static_cast<float>(GetFinalWidth()), static_cast<float>(thumbLength), m_thumbColor, GetOpacity(), clip);
    } else {
        commands.DrawRect(static_cast<float>(absX + thumbPosition), static_cast<float>(absY), static_cast<float>(thumbLength), static_cast<float>(GetFinalHeight()), m_thumbColor, GetOpacity(), clip);
    }
}

}
