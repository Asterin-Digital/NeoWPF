#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>

namespace NeoWPF::UI::Animation {

void Storyboard::AddTimeline(const TimelinePtr& timeline) {
    if (timeline) {
        m_timelines.push_back(timeline);
    }
}

bool Storyboard::Update(float elapsedSeconds) {
    bool completed = true;
    for (const auto& timeline : m_timelines) {
        if (timeline && !timeline->Update(elapsedSeconds)) {
            completed = false;
        }
    }
    return completed;
}

void Storyboard::Clear() {
    m_timelines.clear();
}

}
