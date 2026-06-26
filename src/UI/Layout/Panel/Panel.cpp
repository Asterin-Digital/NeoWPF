#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Core/Visual.h>
#include <algorithm>

namespace NeoWPF::UI {

void Panel::Measure(int availableWidth, int availableHeight)
{
    int maxW = 0;
    int maxH = 0;
    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Measure(availableWidth, availableHeight);
            maxW = (std::max)(maxW, fe->GetDesiredWidth());
            maxH = (std::max)(maxH, fe->GetDesiredHeight());
        }
    }

    if (GetVisualChildren().empty()) {
        SetDesiredSize(availableWidth, availableHeight);
    } else {
        SetDesiredSize(maxW, maxH);
    }
}

void Panel::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Arrange(fe->GetDesiredWidth(), fe->GetDesiredHeight());
            fe->SetLayoutOffset(0, 0);
        }
    }
}

void Panel::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_backgroundColor != 0x00000000u) {
        commands.DrawRect(
            static_cast<float>(GetLayoutOffsetX()),
            static_cast<float>(GetLayoutOffsetY()),
            static_cast<float>(GetFinalWidth()),
            static_cast<float>(GetFinalHeight()),
            m_backgroundColor);
    }

    for (auto child : GetVisualChildren()) {
        if (child) {
            child->BuildRenderCommands(commands);
        }
    }
}

}
