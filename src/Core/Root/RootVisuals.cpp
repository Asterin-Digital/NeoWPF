#include <NeoWPF/Core/Root/RootVisuals.h>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <NeoWPF/UI/Controls/Window.h>

namespace NeoWPF::Core {

namespace {

void BuildVisualTreeCommands(NeoWPF::UI::Visual* visual, NeoWPF::Rendering::CommandList& commands)
{
    if (!visual) {
        return;
    }
    visual->BuildRenderCommands(commands);
    for (NeoWPF::UI::Visual* child : visual->GetVisualChildren()) {
        BuildVisualTreeCommands(child, commands);
    }
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

RootVisualInfo FindRootVisual(const std::vector<NeoWPF::UI::Visual*>& visuals)
{
    RootVisualInfo info;
    for (auto visual : visuals) {
        if (!visual->GetParent()) {
            if (auto window = dynamic_cast<NeoWPF::UI::Window*>(visual)) {
                info.rootWindow = window;
                info.rootVisual = window;
                return info;
            }
            if (!info.rootVisual) {
                info.rootVisual = visual;
            }
        }
    }
    return info;
}

bool AnyTopLevelVisualDirty(const std::vector<NeoWPF::UI::Visual*>& visuals)
{
    for (auto visual : visuals) {
        if (!visual->GetParent() && visual->IsTreeDirty()) {
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
        if (!visual->GetParent()) {
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
        if (!visual->GetParent()) {
            ClearVisualTreeDirty(visual);
        }
    }
}

}
