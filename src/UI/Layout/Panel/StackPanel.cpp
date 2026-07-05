#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <algorithm>

namespace NeoWPF::UI {

void StackPanel::Measure(int availableWidth, int availableHeight)
{
    int totalHeight = 0;
    int maxWidth = 0;
    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Measure(availableWidth, availableHeight);
            const int resolvedW = fe->ResolveWidth(availableWidth, fe->GetDesiredWidth());
            const int resolvedH = fe->ResolveHeight(availableHeight, fe->GetDesiredHeight());
            fe->SetDesiredSize(resolvedW, resolvedH);
            totalHeight += resolvedH;
            maxWidth = (std::max)(maxWidth, resolvedW);
        }
    }

    const int desiredW = ResolveWidth(availableWidth, maxWidth);
    const int desiredH = ResolveHeight(availableHeight, totalHeight);
    SetDesiredSize(desiredW, desiredH);
}

void StackPanel::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    int y = 0;
    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            const int childW = fe->ResolveWidth(finalWidth, fe->GetDesiredWidth());
            const int childH = fe->ResolveHeight(finalHeight, fe->GetDesiredHeight());
            fe->Arrange(childW, childH);
            fe->SetLayoutOffset(0, y);
            y += childH;
        }
    }
}

}
