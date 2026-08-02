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

bool StackPanel::stackPanelDebugLogging = true;

void StackPanel::Measure(int availableWidth, int availableHeight)
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
    int totalMainExtent = 0;
    int maxCrossExtent = 0;
    if (stackPanelDebugLogging) {
        NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] container=%p available=%dx%d", this, availableWidth, availableHeight);
    }

    std::vector<Visual*> seenChildren;
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
            const int childAvailableWidth = availableWidth > 0 ? std::max(0, availableWidth - margin.Horizontal()) : availableWidth;
            const int childAvailableHeight = availableHeight > 0 ? std::max(0, availableHeight - margin.Vertical()) : availableHeight;
            fe->Measure(childAvailableWidth, childAvailableHeight);
            const int childDesiredW = fe->GetDesiredWidth();
            const int childDesiredH = fe->GetDesiredHeight();
            const int resolvedW = std::max(0, fe->ResolveWidth(childAvailableWidth, childDesiredW) + margin.Horizontal());
            const int resolvedH = std::max(0, fe->ResolveHeight(childAvailableHeight, childDesiredH) + margin.Vertical());
            fe->SetDesiredSize(std::max(0, fe->ResolveWidth(childAvailableWidth, childDesiredW)), std::max(0, fe->ResolveHeight(childAvailableHeight, childDesiredH)));
            if (isHorizontal) {
                totalMainExtent += resolvedW;
                maxCrossExtent = (std::max)(maxCrossExtent, resolvedH);
            } else {
                totalMainExtent += resolvedH;
                maxCrossExtent = (std::max)(maxCrossExtent, resolvedW);
            }
            if (stackPanelDebugLogging) {
                NeoWPF::Diagnostics::LogInfo("[StackPanel::Measure] child=%p desired=%dx%d resolved=%dx%d", child, childDesiredW, childDesiredH, resolvedW, resolvedH);
            }
        }
    }

    const int desiredW = isHorizontal
        ? std::max(0, totalMainExtent)
        : std::max(0, maxCrossExtent);
    const int desiredH = isHorizontal
        ? std::max(0, maxCrossExtent)
        : std::max(0, totalMainExtent);
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

    StackPanelGuard guard(this);
    if (guard.IsReentrant()) {
        if (stackPanelDebugLogging) {
            NeoWPF::Diagnostics::LogInfo("[StackPanel::Arrange] reentrant arrange skipped for %p", this);
        }
        return;
    }


    const bool isHorizontal = m_orientation == Orientation::Horizontal;
    int mainOffset = 0;
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
            const int contentWidth = std::max(0, finalWidth - margin.Horizontal());
            const int contentHeight = std::max(0, finalHeight - margin.Vertical());
            const int childW = std::max(0, fe->ResolveWidth(contentWidth, fe->GetDesiredWidth()));
            const int childH = std::max(0, fe->ResolveHeight(contentHeight, fe->GetDesiredHeight()));

            int alignedX = 0;
            int alignedY = 0;
            int alignedW = childW;
            int alignedH = childH;

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
