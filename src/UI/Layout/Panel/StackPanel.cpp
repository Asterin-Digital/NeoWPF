#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <algorithm>

namespace NeoWPF::UI {

void StackPanel::Measure(double availableWidth, double availableHeight)
{
    const bool isHorizontal = m_orientation == Orientation::Horizontal;
    const double itemSpacing = isHorizontal ? GetHorizontalSpacing() : GetVerticalSpacing();
    double totalMainExtent = 0.0;
    double maxCrossExtent = 0.0;

    bool firstChild = true;
    for (auto child : GetVisualChildren()) {
        if (auto uiChild = dynamic_cast<UIElement*>(child); uiChild && !uiChild->IsVisible()) {
            continue;
        }

        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            const auto margin = fe->GetMargin();
            const double childAvailableWidth = availableWidth > 0.0 ? (std::max)(0.0, availableWidth - margin.Horizontal()) : availableWidth;
            const double childAvailableHeight = availableHeight > 0.0 ? (std::max)(0.0, availableHeight - margin.Vertical()) : availableHeight;
            fe->Measure(childAvailableWidth, childAvailableHeight);
            const double childDesiredW = fe->GetDesiredWidth();
            const double childDesiredH = fe->GetDesiredHeight();
            const double resolvedW = (std::max)(0.0, fe->ResolveWidth(childAvailableWidth, childDesiredW) + margin.Horizontal());
            const double resolvedH = (std::max)(0.0, fe->ResolveHeight(childAvailableHeight, childDesiredH) + margin.Vertical());
            fe->SetDesiredSize((std::max)(0.0, fe->ResolveWidth(childAvailableWidth, childDesiredW)), (std::max)(0.0, fe->ResolveHeight(childAvailableHeight, childDesiredH)));
            if (!firstChild) {
                totalMainExtent += itemSpacing;
            }
            if (isHorizontal) {
                totalMainExtent += resolvedW;
                maxCrossExtent = (std::max)(maxCrossExtent, resolvedH);
            } else {
                totalMainExtent += resolvedH;
                maxCrossExtent = (std::max)(maxCrossExtent, resolvedW);
            }
            firstChild = false;
        }
    }

    const double desiredW = isHorizontal
        ? (std::max)(0.0, totalMainExtent)
        : (std::max)(0.0, maxCrossExtent);
    const double desiredH = isHorizontal
        ? (std::max)(0.0, maxCrossExtent)
        : (std::max)(0.0, totalMainExtent);
    SetDesiredSize(desiredW, desiredH);
}

void StackPanel::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);


    const bool isHorizontal = m_orientation == Orientation::Horizontal;
    const double itemSpacing = isHorizontal ? GetHorizontalSpacing() : GetVerticalSpacing();
    double mainOffset = 0.0;
    bool firstChild = true;
    for (auto child : GetVisualChildren()) {
        if (auto uiChild = dynamic_cast<UIElement*>(child); uiChild && !uiChild->IsVisible()) {
            continue;
        }

        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            const auto margin = fe->GetMargin();
            const double contentWidth = (std::max)(0.0, finalWidth - margin.Horizontal());
            const double contentHeight = (std::max)(0.0, finalHeight - margin.Vertical());
            const double childW = (std::max)(0.0, fe->ResolveWidth(contentWidth, fe->GetDesiredWidth()));
            const double childH = (std::max)(0.0, fe->ResolveHeight(contentHeight, fe->GetDesiredHeight()));

            double alignedX = 0.0;
            double alignedY = 0.0;
            double alignedW = childW;
            double alignedH = childH;

            if (!firstChild) {
                mainOffset += itemSpacing;
            }

            if (isHorizontal) {
                Alignment::ApplyAlignment(
                    fe,
                    contentWidth,
                    contentHeight,
                    HorizontalAlignment::Left,
                    fe->GetVerticalAlignment(),
                    alignedX,
                    alignedY,
                    alignedW,
                    alignedH
                );
                alignedX = mainOffset + margin.Left;
                alignedY += margin.Top;
                if (fe->GetVerticalAlignment() != VerticalAlignment::Stretch) {
                    alignedH = childH;
                }
                alignedW = childW;
            } else {
                Alignment::ApplyAlignment(
                    fe,
                    contentWidth,
                    contentHeight,
                    fe->GetHorizontalAlignment(),
                    VerticalAlignment::Top,
                    alignedX,
                    alignedY,
                    alignedW,
                    alignedH
                );
                alignedY = mainOffset + margin.Top;
                alignedX += margin.Left;
                if (fe->GetHorizontalAlignment() != HorizontalAlignment::Stretch) {
                    alignedW = childW;
                }
                alignedH = childH;
            }

            fe->Arrange(alignedW, alignedH);
            fe->SetLayoutOffset(alignedX, alignedY);
            mainOffset += isHorizontal ? childW + margin.Horizontal() : childH + margin.Vertical();
            firstChild = false;
        }
    }
}

void StackPanel::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!IsVisible()) {
        return;
    }

    for (auto child : GetVisualChildrenInZOrder()) {
        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        child->BuildRenderCommands(commands);
    }
}

}
