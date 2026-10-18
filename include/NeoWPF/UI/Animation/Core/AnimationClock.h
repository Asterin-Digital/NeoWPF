/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Animation/Definitions/AnimationTimeline.h>

#include <memory>

namespace NeoWPF::UI::Properties {
class DependencyObject;
class DependencyProperty;
}

namespace NeoWPF::UI::Animation {

class AnimationClock {
public:
    AnimationClock(::NeoWPF::UI::Properties::DependencyObject* target,
                   const ::NeoWPF::UI::Properties::DependencyProperty* targetProperty,
                   std::shared_ptr<AnimationTimeline> timeline);

    bool Tick(float elapsedSeconds);
    void Stop();
    bool IsFinished() const noexcept { return m_finished; }

private:
    void ApplyProgress(double progress);
    int GetIterationCount() const noexcept;

    ::NeoWPF::UI::Properties::DependencyObject* m_target = nullptr;
    const ::NeoWPF::UI::Properties::DependencyProperty* m_targetProperty = nullptr;
    std::shared_ptr<AnimationTimeline> m_timeline;
    float m_elapsedSeconds = 0.0f;
    bool m_finished = false;
};

}
