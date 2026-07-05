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
            const int resolvedW = fe->ResolveWidth(availableWidth, fe->GetDesiredWidth());
            const int resolvedH = fe->ResolveHeight(availableHeight, fe->GetDesiredHeight());
            fe->SetDesiredSize(resolvedW, resolvedH);
            maxW = (std::max)(maxW, resolvedW);
            maxH = (std::max)(maxH, resolvedH);
        }
    }

    const int desiredW = ResolveWidth(availableWidth, maxW);
    const int desiredH = ResolveHeight(availableHeight, maxH);

    if (GetVisualChildren().empty()) {
        SetDesiredSize(desiredW, desiredH);
    } else {
        SetDesiredSize(desiredW, desiredH);
    }
}

void Panel::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            const int resolvedW = fe->ResolveWidth(finalWidth, fe->GetDesiredWidth());
            const int resolvedH = fe->ResolveHeight(finalHeight, fe->GetDesiredHeight());

            int alignedX = 0, alignedY = 0, alignedW = resolvedW, alignedH = resolvedH;
            Alignment::ApplyAlignment(
                fe,
                finalWidth, finalHeight,
                fe->GetHorizontalAlignment(),
                fe->GetVerticalAlignment(),
                alignedX, alignedY, alignedW, alignedH
            );

            if (fe->GetHorizontalAlignment() != HorizontalAlignment::Stretch) {
                alignedW = resolvedW;
            }
            if (fe->GetVerticalAlignment() != VerticalAlignment::Stretch) {
                alignedH = resolvedH;
            }

            fe->Arrange(alignedW, alignedH);
            fe->SetLayoutOffset(alignedX, alignedY);
        }
    }
}

void Panel::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    if (m_backgroundColor != 0x00000000u) {
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            static_cast<float>(GetFinalWidth()),
            static_cast<float>(GetFinalHeight()),
            m_backgroundColor,
            GetOpacity());
    }

    for (auto child : GetVisualChildrenInZOrder()) {
        if (child) {
            child->BuildRenderCommands(commands);
        }
    }
}

}
