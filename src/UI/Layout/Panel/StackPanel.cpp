#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <algorithm>
#include <vector>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace {
    bool IsAncestorInParentChain(NeoWPF::UI::Visual* node, NeoWPF::UI::Visual* possibleAncestor)
    {
        std::vector<NeoWPF::UI::Visual*> visited;
        for (auto parent = node->GetParent(); parent; parent = parent->GetParent()) {
            if (parent == possibleAncestor) {
                return true;
            }
            if (std::find(visited.begin(), visited.end(), parent) != visited.end()) {
                break;
            }
            visited.push_back(parent);
        }
        return false;
    }

    struct StackPanelGuard {
        explicit StackPanelGuard(const NeoWPF::UI::StackPanel* panel)
            : panel(panel), activeStackPanels(GetActiveStackPanels())
        {
            if (std::find(activeStackPanels->begin(), activeStackPanels->end(), panel) != activeStackPanels->end()) {
                alreadyActive = true;
            } else {
                activeStackPanels->push_back(panel);
            }
        }

        ~StackPanelGuard()
        {
            if (!alreadyActive) {
                activeStackPanels->pop_back();
            }
        }

        bool IsReentrant() const noexcept { return alreadyActive; }

    private:
        static std::vector<const NeoWPF::UI::StackPanel*>* GetActiveStackPanels()
        {
            static thread_local std::vector<const NeoWPF::UI::StackPanel*> active;
            return &active;
        }

        const NeoWPF::UI::StackPanel* panel;
        std::vector<const NeoWPF::UI::StackPanel*>* activeStackPanels;
        bool alreadyActive = false;
    };
}

namespace NeoWPF::UI {

bool StackPanel::stackPanelDebugLogging = false;

void StackPanel::Measure(double availableWidth, double availableHeight)
{
    StackPanelGuard guard(this);
    if (guard.IsReentrant()) {
        if (stackPanelDebugLogging) {
            NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] reentrant measure skipped for %p", this);
        }
        SetDesiredSize(0, 0);
        return;
    }

    const bool isHorizontal = m_orientation == Orientation::Horizontal;
    const double itemSpacing = isHorizontal ? GetHorizontalSpacing() : GetVerticalSpacing();
    double totalMainExtent = 0.0;
    double maxCrossExtent = 0.0;
    if (stackPanelDebugLogging) {
        NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] container=%p available=%dx%d spacing=%d", this, availableWidth, availableHeight, itemSpacing);
    }

    std::vector<Visual*> seenChildren;
    bool firstChild = true;
    for (auto child : GetVisualChildren()) {
        if (!child) {
            continue;
        }
        if (std::find(seenChildren.begin(), seenChildren.end(), child) != seenChildren.end()) {
            if (stackPanelDebugLogging) {
                NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] skipping duplicate child=%p", child);
            }
            continue;
        }
        seenChildren.push_back(child);

        if (child == this || IsAncestorInParentChain(this, child)) {
            if (stackPanelDebugLogging) {
                NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] skipping recursive child=%p", child);
            }
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
            if (stackPanelDebugLogging) {
                NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] child=%p desired=%dx%d resolved=%dx%d", child, childDesiredW, childDesiredH, resolvedW, resolvedH);
            }
        }
    }

    const double desiredW = isHorizontal
        ? (std::max)(0.0, totalMainExtent)
        : (std::max)(0.0, maxCrossExtent);
    const double desiredH = isHorizontal
        ? (std::max)(0.0, maxCrossExtent)
        : (std::max)(0.0, totalMainExtent);
    SetDesiredSize(desiredW, desiredH);
    if (stackPanelDebugLogging) {
        NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] result desired=%dx%d", desiredW, desiredH);
    }
}

void StackPanel::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    if (stackPanelDebugLogging) {
        NeoWPF::Diagnostics::LogInfo("[StackPanel::Arrange] container=%p final=%dx%d", this, finalWidth, finalHeight);
    }

    StackPanelGuard guard(this);
    if (guard.IsReentrant()) {
        if (stackPanelDebugLogging) {
            NeoWPF::Diagnostics::LogInfo("[StackPanel::Arrange] reentrant arrange skipped for %p", this);
        }
        return;
    }


    const bool isHorizontal = m_orientation == Orientation::Horizontal;
    const double itemSpacing = isHorizontal ? GetHorizontalSpacing() : GetVerticalSpacing();
    double mainOffset = 0.0;
    bool firstChild = true;
    std::vector<Visual*> seenChildren;
    for (auto child : GetVisualChildren()) {
        if (!child) {
            continue;
        }
        if (std::find(seenChildren.begin(), seenChildren.end(), child) != seenChildren.end()) {

            if (stackPanelDebugLogging) {
                NeoWPF::Diagnostics::LogInfo("[StackPanel::Arrange] skipping duplicate child=%p", child);
            }
            continue;
        }
        seenChildren.push_back(child);

        if (child == this || IsAncestorInParentChain(this, child)) {
            if (stackPanelDebugLogging) {
                NeoWPF::Diagnostics::LogInfo("[StackPanel::Arrange] skipping recursive child=%p", child);
            }
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
                if (fe->GetHorizontalAlignment() != HorizontalAlignment::Stretch) {
                    alignedW = childW;
                }
                alignedH = childH;
            }

            fe->Arrange(alignedW, alignedH);
            fe->SetLayoutOffset(alignedX, alignedY);
            if (stackPanelDebugLogging) {
                NeoWPF::Diagnostics::LogInfo("[StackPanel::Arrange] child=%p offset=%d size=%dx%d desired=%dx%d aligned=%dx%d@%d,%d", child, mainOffset, childW, childH, fe->GetDesiredWidth(), fe->GetDesiredHeight(), alignedW, alignedH, alignedX, alignedY);
            }
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
        if (!child) {
            continue;
        }

        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        child->BuildRenderCommands(commands);
    }
}

}
