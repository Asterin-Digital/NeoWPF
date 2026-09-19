#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/PanelStyles.h>
#include <NeoWPF/ControlStyles.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <algorithm>
#include <vector>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

Panel::Panel()
{
    APPLY_CONTROL_STYLES(NeoWPF::UI::Generated::PanelStyles, *this);
}

namespace {
int ClampToDesired(int value, int desired)
{
    return std::min(value, desired);
}

bool IsAncestorInParentChain(NeoWPF::UI::Visual* node, NeoWPF::UI::Visual* possibleAncestor)
{
    for (auto parent = node->GetParent(); parent; parent = parent->GetParent()) {
        if (parent == possibleAncestor) {
            return true;
        }
    }
    return false;
}

struct ActivePanelGuard {
    explicit ActivePanelGuard(const Panel* panel)
        : panel(panel), activePanels(GetActivePanels())
    {
        if (std::find(activePanels->begin(), activePanels->end(), panel) != activePanels->end()) {
            alreadyActive = true;
        } else {
            activePanels->push_back(panel);
        }
    }

    ~ActivePanelGuard()
    {
        if (!alreadyActive) {
            activePanels->pop_back();
        }
    }

    bool IsReentrant() const noexcept { return alreadyActive; }
    bool IsActive(const Panel* childPanel) const noexcept
    {
        return std::find(activePanels->begin(), activePanels->end(), childPanel) != activePanels->end();
    }

private:
    static std::vector<const Panel*>* GetActivePanels()
    {
        static thread_local std::vector<const Panel*> active;
        return &active;
    }

    const Panel* panel;
    std::vector<const Panel*>* activePanels;
    bool alreadyActive = false;
};

struct ActiveVisualGuard {
    explicit ActiveVisualGuard(const Visual* visual)
        : visual(visual), activeVisuals(GetActiveVisuals())
    {
        if (std::find(activeVisuals->begin(), activeVisuals->end(), visual) != activeVisuals->end()) {
            alreadyActive = true;
        } else {
            activeVisuals->push_back(visual);
        }
    }

    ~ActiveVisualGuard()
    {
        if (!alreadyActive) {
            activeVisuals->pop_back();
        }
    }

    bool IsReentrant() const noexcept { return alreadyActive; }
    bool IsActive(const Visual* child) const noexcept
    {
        return std::find(activeVisuals->begin(), activeVisuals->end(), child) != activeVisuals->end();
    }

private:
    static std::vector<const Visual*>* GetActiveVisuals()
    {
        static thread_local std::vector<const Visual*> active;
        return &active;
    }

    const Visual* visual;
    std::vector<const Visual*>* activeVisuals;
    bool alreadyActive = false;
};
}

void Panel::Measure(double availableWidth, double availableHeight)
{
    ActivePanelGuard guard(this);
    if (guard.IsReentrant()) {
        SetDesiredSize(0, 0);
        return;
    }

    double maxW = 0.0;
    double maxH = 0.0;
    for (auto child : GetLogicalChildren()) {
        if (child == this || IsAncestorInParentChain(this, child)) {
            continue;
        }

        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            if (auto childPanel = dynamic_cast<Panel*>(fe)) {
                if (guard.IsActive(childPanel)) {
                    continue;
                }
            }
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

    if (GetLogicalChildren().empty()) {
        SetDesiredSize(desiredW, desiredH);
    } else {
        SetDesiredSize(desiredW, desiredH);
    }
}

void Panel::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    ActivePanelGuard guard(this);
    if (guard.IsReentrant()) {
        return;
    }

    for (auto child : GetLogicalChildren()) {
        if (child == this || IsAncestorInParentChain(this, child)) {
            continue;
        }

        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            if (auto childPanel = dynamic_cast<Panel*>(fe)) {
                if (guard.IsActive(childPanel)) {
                    continue;
                }
            }
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

    ActiveVisualGuard renderGuard(this);
    if (renderGuard.IsReentrant()) {
        return;
    }

    double absX = 0.0;
    double absY = 0.0;
    GetAbsolutePosition(absX, absY);

    if (GetBackgroundColor().ToArgb() != 0x00000000u) {
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
            GetBackgroundColor().ToArgb(),
            GetOpacity());
    }

    for (auto child : GetVisualChildrenInZOrder()) {
        if (!child) {
            continue;
        }

        if (renderGuard.IsActive(child)) {
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

void Panel::SetBackgroundColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBackgroundColor(color);
    InvalidateVisual();
}

NeoWPF::UI::Color Panel::GetBackgroundColor() const noexcept
{
    return Control::GetBackgroundColor();
}

void Panel::OnBackgroundColorChanged(NeoWPF::UI::Color)
{
    InvalidateVisual();
}

}
