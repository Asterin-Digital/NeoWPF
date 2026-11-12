#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <algorithm>

namespace NeoWPF::UI {

Panel::Panel() = default;

void Panel::Measure(double availableWidth, double availableHeight)
{
    double maxW = 0.0;
    double maxH = 0.0;
    for (auto child : GetVisualChildren()) {
        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            const auto margin = fe->GetMargin();
            const double childAvailableWidth = availableWidth > 0.0 ? (std::max)(0.0, availableWidth - margin.Horizontal()) : availableWidth;
            const double childAvailableHeight = availableHeight > 0.0 ? (std::max)(0.0, availableHeight - margin.Vertical()) : availableHeight;
            fe->Measure(childAvailableWidth, childAvailableHeight);
            const double resolvedW = fe->ResolveWidth(childAvailableWidth, fe->GetDesiredWidth()) + margin.Horizontal();
            const double resolvedH = fe->ResolveHeight(childAvailableHeight, fe->GetDesiredHeight()) + margin.Vertical();
            fe->SetDesiredSize((std::max)(0.0, fe->ResolveWidth(childAvailableWidth, fe->GetDesiredWidth())), (std::max)(0.0, fe->ResolveHeight(childAvailableHeight, fe->GetDesiredHeight())));
            maxW = (std::max)(maxW, resolvedW);
            maxH = (std::max)(maxH, resolvedH);
        }
    }

    const double desiredW = ResolveWidth(availableWidth, maxW);
    const double desiredH = ResolveHeight(availableHeight, maxH);

    SetDesiredSize(desiredW, desiredH);
}

void Panel::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    for (auto child : GetVisualChildren()) {
        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            const auto margin = fe->GetMargin();
            const double contentWidth = (std::max)(0.0, finalWidth - margin.Horizontal());
            const double contentHeight = (std::max)(0.0, finalHeight - margin.Vertical());
            const double resolvedW = (std::max)(0.0, fe->ResolveWidth(contentWidth, fe->GetDesiredWidth()));
            const double resolvedH = (std::max)(0.0, fe->ResolveHeight(contentHeight, fe->GetDesiredHeight()));

            double alignedX = 0.0, alignedY = 0.0, alignedW = resolvedW, alignedH = resolvedH;
            Alignment::ApplyAlignment(
                fe,
                contentWidth, contentHeight,
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
            fe->SetLayoutOffset(alignedX + margin.Left, alignedY + margin.Top);
        }
    }
}

void Panel::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!IsVisible()) {
        return;
    }

    double absX = 0.0;
    double absY = 0.0;
    GetAbsolutePosition(absX, absY);

    if (const auto backgroundBrush = std::dynamic_pointer_cast<SolidColorBrush>(GetBackgroundBrush());
        backgroundBrush && backgroundBrush->GetArgb() != 0x00000000u) {
        const auto cornerRadius = GetCornerRadius();
        commands.DrawRoundedRect(
            absX,
            absY,
            GetFinalWidth(),
            GetFinalHeight(),
            cornerRadius.TopLeft,
            cornerRadius.TopRight,
            cornerRadius.BottomRight,
            cornerRadius.BottomLeft,
                NeoWPF::Rendering::Paint::Solid(backgroundBrush->GetArgb()),
                static_cast<float>(GetOpacity() * backgroundBrush->GetOpacity()));
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
