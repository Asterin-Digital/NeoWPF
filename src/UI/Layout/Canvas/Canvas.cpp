#include <NeoWPF/UI/Layout/Canvas/Canvas.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <algorithm>
#include <cmath>

namespace NeoWPF::UI {

namespace {

constexpr double kUnboundedMeasure = 1000000.0;

bool IsSet(double value) noexcept
{
    return !std::isnan(value);
}

double ResolvePosition(double leading, double trailing, double available, double childSize) noexcept
{
    if (IsSet(leading)) {
        return leading;
    }
    if (IsSet(trailing)) {
        return available - trailing - childSize;
    }
    return 0.0;
}

}

Canvas::Canvas() noexcept
{
}

void Canvas::Measure(double availableWidth, double availableHeight)
{
    for (auto child : GetVisualChildren()) {
        if (!child) {
            continue;
        }

        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        if (auto frameworkChild = dynamic_cast<FrameworkElement*>(child)) {
            frameworkChild->Measure(kUnboundedMeasure, kUnboundedMeasure);
        }
    }

    SetDesiredSize(
        ResolveWidth(availableWidth, 0.0),
        ResolveHeight(availableHeight, 0.0));
}

void Canvas::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    for (auto child : GetVisualChildren()) {
        if (!child) {
            continue;
        }

        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        auto frameworkChild = dynamic_cast<FrameworkElement*>(child);
        if (!frameworkChild) {
            continue;
        }

        const auto margin = frameworkChild->GetMargin();
        const double contentWidth = (std::max)(0.0, finalWidth - margin.Horizontal());
        const double contentHeight = (std::max)(0.0, finalHeight - margin.Vertical());
        const double childWidth = (std::max)(0.0, frameworkChild->ResolveWidth(contentWidth, frameworkChild->GetDesiredWidth()));
        const double childHeight = (std::max)(0.0, frameworkChild->ResolveHeight(contentHeight, frameworkChild->GetDesiredHeight()));

        frameworkChild->Arrange(childWidth, childHeight);

        const double x = ResolvePosition(GetLeft(child), GetRight(child), finalWidth, childWidth) + margin.Left;
        const double y = ResolvePosition(GetTop(child), GetBottom(child), finalHeight, childHeight) + margin.Top;
        frameworkChild->SetLayoutOffset(x, y);
    }
}

}