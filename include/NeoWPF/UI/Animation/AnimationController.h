#pragma once

#include <memory>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>

namespace NeoWPF::UI {

class UIElement;
class FrameworkElement;
namespace Animation { class Storyboard; }

void AddStoryboard(std::shared_ptr<Animation::Storyboard> storyboard);
void Tick();

}
