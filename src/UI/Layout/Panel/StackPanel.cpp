#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Layout/Alignment/Alignment.h>
#include <algorithm>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

bool StackPanel::stackPanelDebugLogging = true; // dont turn this on unless ur debugging something

void StackPanel::Measure(int availableWidth, int availableHeight)
{
    int totalHeight = 0;
    int maxWidth = 0;
    if (stackPanelDebugLogging) {
        NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] container=%p available=%dx%d", this, availableWidth, availableHeight);
    }
    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Measure(availableWidth, availableHeight);
            const int childDesiredW = fe->GetDesiredWidth();
            const int childDesiredH = fe->GetDesiredHeight();
            const int resolvedW = fe->ResolveWidth(availableWidth, childDesiredW);
            const int resolvedH = fe->ResolveHeight(availableHeight, childDesiredH);
            fe->SetDesiredSize(resolvedW, resolvedH);
            totalHeight += resolvedH;
            maxWidth = (std::max)(maxWidth, resolvedW);
            if (stackPanelDebugLogging) {
                NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] child=%p desired=%dx%d resolved=%dx%d", child, childDesiredW, childDesiredH, resolvedW, resolvedH);
            }
        }
    }

    const int desiredW = std::max(0, std::min(availableWidth, maxWidth));
    const int desiredH = std::max(0, std::min(availableHeight, totalHeight));
    SetDesiredSize(desiredW, desiredH);
    if (stackPanelDebugLogging) {
        NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] result desired=%dx%d", desiredW, desiredH);
    }
}

void StackPanel::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    if (stackPanelDebugLogging) {
        NeoWPF::Diagnostics::LogInfo("[StackPanel::Arrange] container=%p final=%dx%d", this, finalWidth, finalHeight);
    }

    int y = 0;
    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            const int childW = fe->ResolveWidth(finalWidth, fe->GetDesiredWidth());
            const int childH = fe->ResolveHeight(finalHeight, fe->GetDesiredHeight());

            int alignedX = 0;
            int alignedY = y;
            int alignedW = childW;
            int alignedH = childH;
            Alignment::ApplyAlignment(
                fe,
                finalWidth,
                finalHeight,
                fe->GetHorizontalAlignment(),
                VerticalAlignment::Top,
                alignedX,
                alignedY,
                alignedW,
                alignedH
            );
            alignedY = y;

            if (fe->GetHorizontalAlignment() != HorizontalAlignment::Stretch) {
                alignedW = childW;
            }
            alignedH = childH;

            fe->Arrange(alignedW, alignedH);
            fe->SetLayoutOffset(alignedX, alignedY);
            if (stackPanelDebugLogging) {
                NeoWPF::Diagnostics::LogInfo("[StackPanel::Arrange] child=%p y=%d size=%dx%d desired=%dx%d aligned=%dx%d@%d,%d", child, y, childW, childH, fe->GetDesiredWidth(), fe->GetDesiredHeight(), alignedW, alignedH, alignedX, alignedY);
            }
            y += childH;
        }
    }
}

}
