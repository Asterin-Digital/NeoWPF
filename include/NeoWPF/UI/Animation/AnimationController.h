/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

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
