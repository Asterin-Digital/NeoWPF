#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Layout/Alignment/Alignment.h>
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

bool StackPanel::stackPanelDebugLogging = true; // dont turn this on unless ur debugging something

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

    int totalHeight = 0;
    int maxWidth = 0;
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
    const int desiredH = totalHeight;
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

    int y = 0;
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
