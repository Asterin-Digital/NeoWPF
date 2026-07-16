#pragma once

#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <vector>

namespace NeoWPF::UI {
    class FrameworkElement;
    class Window;
    class Visual;
}

namespace NeoWPF::Core {

struct RootVisualInfo {
    NeoWPF::UI::Visual* rootVisual = nullptr;
    NeoWPF::UI::Window* rootWindow = nullptr;
};

RootVisualInfo FindRootVisual(const std::vector<NeoWPF::UI::Visual*>& visuals);
bool AnyTopLevelVisualDirty(const std::vector<NeoWPF::UI::Visual*>& visuals);
void LayoutAndBuildVisualTree(NeoWPF::UI::Visual* rootVisual, unsigned int width, unsigned int height, NeoWPF::Rendering::CommandList& buildCommandList);
void LayoutAndBuildAllTopLevelVisuals(const std::vector<NeoWPF::UI::Visual*>& visuals, unsigned int width, unsigned int height, NeoWPF::Rendering::CommandList& buildCommandList);
void ClearTopLevelVisualsDirty(const std::vector<NeoWPF::UI::Visual*>& visuals);
void ClearVisualTreeDirtyForRoot(NeoWPF::UI::Visual* rootVisual);

}
