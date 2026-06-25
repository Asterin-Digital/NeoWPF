#include <NeoWPF/UI/Layout/Panel.h>
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
    SetDesiredSize(maxW, maxH);
}

void Panel::Arrange(int finalWidth, int finalHeight)
{

    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Arrange(fe->GetDesiredWidth(), fe->GetDesiredHeight());
            fe->SetLayoutOffset(0, 0);
        }
    }
}

}
