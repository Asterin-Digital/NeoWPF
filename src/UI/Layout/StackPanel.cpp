#include <NeoWPF/UI/Layout/StackPanel.h>
#include <NeoWPF/UI/Core/Visual.h>
#include <algorithm>

namespace NeoWPF::UI {

void StackPanel::Measure(int availableWidth, int availableHeight)
{
    int totalHeight = 0;
    int maxWidth = 0;
    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Measure(availableWidth, availableHeight);
            totalHeight += fe->GetDesiredHeight();
            maxWidth = (std::max)(maxWidth, fe->GetDesiredWidth());
        }
    }
    SetDesiredSize(maxWidth, totalHeight);
}

void StackPanel::Arrange(int finalWidth, int finalHeight)
{
    int y = 0;
    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            int childW = finalWidth;
            int childH = fe->GetDesiredHeight();
            fe->Arrange(childW, childH);
            fe->SetLayoutOffset(0, y);
            y += childH;
        }
    }
}

}
