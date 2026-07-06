#pragma once

#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <memory>
#include <string>
#include <vector>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Styling {

class VisualState;

namespace detail {
bool ApplyVisualStateTransition(UIElement* element, const std::shared_ptr<VisualState>& state, bool useTransitions);
}

class VisualState {
public:
    explicit VisualState(std::string name = {});

    void SetName(std::string name) noexcept;
    const std::string& GetName() const noexcept;

    void SetTargetName(std::string targetName) noexcept;
    const std::string& GetTargetName() const noexcept;

    void SetStoryboard(const std::shared_ptr<NeoWPF::UI::Animation::Storyboard>& storyboard) noexcept;
    const std::shared_ptr<NeoWPF::UI::Animation::Storyboard>& GetStoryboard() const noexcept;
    bool HasStoryboard() const noexcept;

private:
    std::string m_name;
    std::string m_targetName;
    std::shared_ptr<NeoWPF::UI::Animation::Storyboard> m_storyboard;
};

class VisualStateGroup {
public:
    explicit VisualStateGroup(std::string name = {});

    void SetName(std::string name) noexcept;
    const std::string& GetName() const noexcept;

    void AddState(const std::shared_ptr<VisualState>& state);
    const std::vector<std::shared_ptr<VisualState>>& GetStates() const noexcept;
    std::shared_ptr<VisualState> FindState(const std::string& name) const;

    std::shared_ptr<VisualState> GetCurrentState() const noexcept;
    void SetCurrentState(const std::shared_ptr<VisualState>& state) noexcept;

private:
    std::string m_name;
    std::vector<std::shared_ptr<VisualState>> m_states;
    std::shared_ptr<VisualState> m_currentState;
};

class VisualStateManager {
public:
    void AddGroup(const std::shared_ptr<VisualStateGroup>& group);
    const std::vector<std::shared_ptr<VisualStateGroup>>& GetGroups() const noexcept;

    std::shared_ptr<VisualStateGroup> FindGroup(const std::string& name) const;
    std::shared_ptr<VisualState> FindState(const std::string& groupName, const std::string& stateName) const;

    bool GoToState(UIElement* element, const std::string& groupName, const std::string& stateName, bool useTransitions = true);
    bool GoToState(UIElement* element, const std::string& stateName, bool useTransitions = true);

private:
    std::vector<std::shared_ptr<VisualStateGroup>> m_groups;
};

}
