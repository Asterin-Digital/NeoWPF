#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBarTemplate.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>
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
}

ScrollBar::~ScrollBar() = default;

void ScrollBar::Measure(int availableWidth, int availableHeight)
{
    SetDesiredSize(m_isVertical ? 16 : 120, m_isVertical ? 120 : 16);
}

void ScrollBar::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
}

void ScrollBar::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX, absY;
    GetAbsolutePosition(absX, absY);

    auto* track = dynamic_cast<Templates::TemplateVisual*>(FindTemplateChild("PART_Track"));
    auto* thumb = dynamic_cast<Templates::TemplateVisual*>(FindTemplateChild("PART_Thumb"));

    if (track) {
        track->SetRect(absX, absY, GetFinalWidth(), GetFinalHeight());
        track->SetFillColor(m_trackColor);
        track->SetOpacity(GetOpacity());
        track->BuildRenderCommands(commands);
    } else {
        commands.DrawRect(static_cast<float>(absX), static_cast<float>(absY), static_cast<float>(GetFinalWidth()), static_cast<float>(GetFinalHeight()), m_trackColor, GetOpacity());
    }

    const int thumbSize = std::max(8, std::min(m_thumbSize, m_isVertical ? GetFinalHeight() : GetFinalWidth()));
    const int thumbOffset = std::max(0, (m_isVertical ? GetFinalHeight() : GetFinalWidth()) / 8);

    if (thumb) {
        if (m_isVertical) {
            thumb->SetRect(absX, absY + thumbOffset, GetFinalWidth(), thumbSize);
        } else {
            thumb->SetRect(absX + thumbOffset, absY, thumbSize, GetFinalHeight());
        }
        thumb->SetFillColor(m_thumbColor);
        thumb->SetOpacity(GetOpacity());
        thumb->BuildRenderCommands(commands);
    } else if (m_isVertical) {
        commands.DrawRect(static_cast<float>(absX), static_cast<float>(absY + thumbOffset), static_cast<float>(GetFinalWidth()), static_cast<float>(thumbSize), m_thumbColor, GetOpacity());
    } else {
        commands.DrawRect(static_cast<float>(absX + thumbOffset), static_cast<float>(absY), static_cast<float>(thumbSize), static_cast<float>(GetFinalHeight()), m_thumbColor, GetOpacity());
    }
}

}
