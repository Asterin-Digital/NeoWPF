#pragma once

#include <NeoWPF/UI/Animation/Definitions/AnimationTimeline.h>
#include <memory>
#include <string>
#include <vector>

namespace NeoWPF::UI::Animation {

struct StoryboardChild {
    std::string TargetName;
    std::string TargetPropertyName;
    std::shared_ptr<AnimationTimeline> Timeline;
};

class Storyboard {
public:
    void AddChild(const std::string& targetName,
                  const std::string& targetPropertyName,
                  std::shared_ptr<AnimationTimeline> timeline);
    const std::vector<StoryboardChild>& GetChildren() const noexcept { return m_children; }
    void Clear();

private:
    std::vector<StoryboardChild> m_children;
};

}
