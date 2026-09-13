#pragma once

#include <NeoWPF/UI/Animation/Definitions/AnimationTimeline.h>
#include <memory>
#include <string>
#include <vector>

namespace NeoWPF::UI::Animation {

/*
    @summary
    Describes an animation assigned to a named target.
*/
struct StoryboardChild {
    std::string TargetName;
    std::string TargetPropertyName;
    std::shared_ptr<AnimationTimeline> Timeline;
};

/*
    @summary
    Groups animations for coordinated playback.
*/
class Storyboard {
public:
    /*
        @summary
        Adds an animation to the storyboard.

        @param targetName
        Target element name.

        @param targetPropertyName
        Target property name.

        @param timeline
        Animation timeline.
    */
    void AddChild(const std::string& targetName,
                  const std::string& targetPropertyName,
                  std::shared_ptr<AnimationTimeline> timeline);
    const std::vector<StoryboardChild>& GetChildren() const noexcept { return m_children; }
    /*
        @summary
        Removes all storyboard animations.
    */
    void Clear();

private:
    std::vector<StoryboardChild> m_children;
};

}
