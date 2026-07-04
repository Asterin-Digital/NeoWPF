#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Layout/Alignment/Alignment.h>
#include <algorithm>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

namespace {
int ClampToDesired(int value, int desired)
{
    return std::min(value, desired);
}
}

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
        SetDesiredSize(0, 0);
    } else {
        SetDesiredSize(maxW, maxH);
    }
}

void Panel::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {

            int alignedX = 0, alignedY = 0, alignedW = fe->GetDesiredWidth(), alignedH = fe->GetDesiredHeight();
            Alignment::ApplyAlignment(
                fe,
                finalWidth, finalHeight,
                fe->GetHorizontalAlignment(),
                fe->GetVerticalAlignment(),
                alignedX, alignedY, alignedW, alignedH
            );

            if (fe->GetHorizontalAlignment() != HorizontalAlignment::Stretch) {
                alignedW = fe->GetDesiredWidth();
            }
            if (fe->GetVerticalAlignment() != VerticalAlignment::Stretch) {
                alignedH = fe->GetDesiredHeight();
            }

            fe->Arrange(alignedW, alignedH);
            fe->SetLayoutOffset(alignedX, alignedY);
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
