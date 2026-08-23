#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>

namespace NeoWPF::UI {

class UIElement;
class FrameworkElement;
namespace Animation { class Storyboard; }

using AnimationHandle = std::uint64_t;
inline constexpr AnimationHandle kInvalidAnimationHandle = 0;

AnimationHandle BeginStoryboard(std::shared_ptr<Animation::Storyboard> storyboard, UIElement* target);
AnimationHandle BeginValueAnimation(float fromValue,
									float toValue,
									float durationSeconds,
									std::function<void(float)> callback);
void StopAnimation(AnimationHandle handle);
void StopStoryboard(AnimationHandle handle);
void Tick();

}
