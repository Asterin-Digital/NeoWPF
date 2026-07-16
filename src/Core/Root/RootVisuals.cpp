#include <NeoWPF/Core/Root/RootVisuals.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Controls/Window.h>

namespace NeoWPF::Core {

namespace {

bool IsSyntheticWindowHost(NeoWPF::UI::Visual* visual)
{
    return dynamic_cast<NeoWPF::UI::Window*>(visual) != nullptr;
}

void BuildVisualTreeCommands(NeoWPF::UI::Visual* visual, NeoWPF::Rendering::CommandList& commands)
{
    if (!visual) {
        return;
    }
    visual->BuildRenderCommands(commands);
}

void ClearVisualTreeDirty(NeoWPF::UI::Visual* visual)
{
    if (!visual) {
        return;
    }

    visual->ClearTreeDirty();
    if (auto uiElement = dynamic_cast<NeoWPF::UI::UIElement*>(visual)) {
        uiElement->ClearDirty();
    }

    for (NeoWPF::UI::Visual* child : visual->GetVisualChildren()) {
        ClearVisualTreeDirty(child);
    }
}

}

void ClearVisualTreeDirtyForRoot(NeoWPF::UI::Visual* rootVisual)
{
    ClearVisualTreeDirty(rootVisual);
}

RootVisualInfo FindRootVisual(const std::vector<NeoWPF::UI::Visual*>& visuals)
{
    RootVisualInfo info;
    for (auto visual : visuals) {
        if (!visual->GetParent() && !IsSyntheticWindowHost(visual)) {
            info.rootVisual = visual;
            return info;
        }
        if (!visual->GetParent() && IsSyntheticWindowHost(visual)) {
            info.rootWindow = dynamic_cast<NeoWPF::UI::Window*>(visual);
        }
    }

    for (auto visual : visuals) {
        if (!visual->GetParent()) {
            info.rootVisual = visual;
            info.rootWindow = dynamic_cast<NeoWPF::UI::Window*>(visual);
            return info;
        }
    }

    return info;
}

bool AnyTopLevelVisualDirty(const std::vector<NeoWPF::UI::Visual*>& visuals)
{
    for (auto visual : visuals) {
        if (!visual->GetParent() && !IsSyntheticWindowHost(visual) && visual->IsTreeDirty()) {
            return true;
        }
    }
    return false;
}

void LayoutAndBuildVisualTree(NeoWPF::UI::Visual* rootVisual, unsigned int width, unsigned int height, NeoWPF::Rendering::CommandList& buildCommandList)
{
    if (!rootVisual) {
        return;
    }

    if (auto fe = dynamic_cast<NeoWPF::UI::FrameworkElement*>(rootVisual)) {
        fe->Measure(static_cast<int>(width), static_cast<int>(height));
        fe->Arrange(static_cast<int>(width), static_cast<int>(height));
    }
    BuildVisualTreeCommands(rootVisual, buildCommandList);
}

void LayoutAndBuildAllTopLevelVisuals(const std::vector<NeoWPF::UI::Visual*>& visuals, unsigned int width, unsigned int height, NeoWPF::Rendering::CommandList& buildCommandList)
{
    for (auto visual : visuals) {
        if (!visual->GetParent() && !IsSyntheticWindowHost(visual)) {
            if (auto fe = dynamic_cast<NeoWPF::UI::FrameworkElement*>(visual)) {
                fe->Measure(static_cast<int>(width), static_cast<int>(height));
                fe->Arrange(static_cast<int>(width), static_cast<int>(height));
            }
            BuildVisualTreeCommands(visual, buildCommandList);
        }
    }
}

void ClearTopLevelVisualsDirty(const std::vector<NeoWPF::UI::Visual*>& visuals)
{
    for (auto visual : visuals) {
        if (!visual->GetParent() && !IsSyntheticWindowHost(visual)) {
            ClearVisualTreeDirty(visual);
        }
    }
}

}
