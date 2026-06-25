#include <NeoWPF/UI/Layout/Panel.h>
#include <NeoWPF/UI/Core/Visual.h>
#include <algorithm>

namespace NeoWPF::UI {

void Panel::Measure(int availableWidth, int availableHeight)
{
    // Default: measure children with same available size and take max desired size
    int maxW = 0;
    int maxH = 0;
    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Measure(availableWidth, availableHeight);
            maxW = (std::max)(maxW, fe->GetDesiredWidth());
            maxH = (std::max)(maxH, fe->GetDesiredHeight());
        }
    }
    SetDesiredSize(maxW, maxH);
}

void Panel::Arrange(int finalWidth, int finalHeight)
{
    // Default: arrange children at origin with their desired size
    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Arrange(fe->GetDesiredWidth(), fe->GetDesiredHeight());
            fe->SetLayoutOffset(0, 0);
        }
    }
    // store final size via FrameworkElement's Arrange
    // (FrameworkElement already stores m_finalWidth/m_finalHeight via Arrange override)
}

}
