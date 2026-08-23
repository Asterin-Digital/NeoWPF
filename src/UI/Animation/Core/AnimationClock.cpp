#include <NeoWPF/UI/Animation/Core/AnimationClock.h>

#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace NeoWPF::UI::Animation {

AnimationClock::AnimationClock(::NeoWPF::UI::Properties::DependencyObject* target,
                               const ::NeoWPF::UI::Properties::DependencyProperty* targetProperty,
                               std::shared_ptr<AnimationTimeline> timeline)
    : m_target(target)
    , m_targetProperty(targetProperty)
    , m_timeline(std::move(timeline))
{
}

int AnimationClock::GetIterationCount() const noexcept {
    if (!m_timeline) {
        return 0;
    }
    switch (m_timeline->GetRepeatBehavior()) {
    case RepeatBehavior::Count:
        return m_timeline->GetRepeatCount();
    case RepeatBehavior::Loop:
        return std::numeric_limits<int>::max();
    case RepeatBehavior::Once:
    default:
        return 1;
    }
}

void AnimationClock::ApplyProgress(double progress) {
    if (!m_target || !m_targetProperty || !m_timeline) {
        return;
    }
    m_target->SetValueAnimated(*m_targetProperty, m_timeline->GetCurrentValue(progress));
}

bool AnimationClock::Tick(float elapsedSeconds) {
    if (m_finished || !m_timeline || !m_target || !m_targetProperty) {
        m_finished = true;
        return true;
    }

    m_elapsedSeconds += std::max(0.0f, elapsedSeconds);
    const float beginTime = m_timeline->GetBeginTime();
    if (m_elapsedSeconds < beginTime) {
        return false;
    }

    const float duration = m_timeline->GetDuration();
    const int iterationCount = GetIterationCount();
    const float activeSeconds = m_elapsedSeconds - beginTime;
    if (duration <= 0.0f) {
        ApplyProgress(m_timeline->GetAutoReverse() ? 0.0 : 1.0);
        m_finished = true;
        if (m_timeline->GetFillBehavior() == FillBehavior::Stop) {
            m_target->ClearAnimatedValue(*m_targetProperty);
        }
        return m_finished;
    }

    const float iterationDuration = m_timeline->GetAutoReverse() ? duration * 2.0f : duration;
    const float totalDuration = iterationDuration * static_cast<float>(iterationCount);
    const bool completed = iterationCount != std::numeric_limits<int>::max() && activeSeconds >= totalDuration;
    const float clampedSeconds = completed ? totalDuration : activeSeconds;
    const int iteration = std::min(static_cast<int>(clampedSeconds / iterationDuration), iterationCount - 1);
    float iterationProgress = (clampedSeconds - iteration * iterationDuration) / iterationDuration;
    if (m_timeline->GetAutoReverse() && iterationProgress > 0.5f) {
        iterationProgress = 1.0f - (iterationProgress - 0.5f) * 2.0f;
    } else if (m_timeline->GetAutoReverse()) {
        iterationProgress *= 2.0f;
    }

    ApplyProgress(std::clamp(static_cast<double>(iterationProgress), 0.0, 1.0));
    if (completed) {
        m_finished = true;
        if (m_timeline->GetFillBehavior() == FillBehavior::Stop) {
            m_target->ClearAnimatedValue(*m_targetProperty);
        }
    }
    return completed;
}

void AnimationClock::Stop() {
    if (m_finished) {
        return;
    }
    if (m_target && m_targetProperty && m_timeline && m_timeline->GetFillBehavior() == FillBehavior::Stop) {
        m_target->ClearAnimatedValue(*m_targetProperty);
    }
    m_finished = true;
}

}
