#pragma once

#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <memory>
#include <vector>

namespace NeoWPF::UI::Animation {

class Storyboard {
public:
    using TimelinePtr = std::shared_ptr<Timeline>;

    void AddTimeline(const TimelinePtr& timeline);
    bool Update(float elapsedSeconds);
    void Clear();

private:
    std::vector<TimelinePtr> m_timelines;
};

}
