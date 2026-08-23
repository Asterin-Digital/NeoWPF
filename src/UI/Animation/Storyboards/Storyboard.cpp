#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>

namespace NeoWPF::UI::Animation {

void Storyboard::AddChild(const std::string& targetName,
                          const std::string& targetPropertyName,
                          std::shared_ptr<AnimationTimeline> timeline) {
    if (timeline) {
        m_children.push_back({targetName, targetPropertyName, std::move(timeline)});
    }
}

void Storyboard::Clear() {
    m_children.clear();
}

}
