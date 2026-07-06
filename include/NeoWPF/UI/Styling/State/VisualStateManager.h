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
/*
    @summary
    Applies the transition logic for a visual state.

    @param element
    The element that owns or hosts the visual state.

    @param state
    The visual state being activated.

    @param useTransitions
    Whether storyboard transitions should be played.

    @returns
    True when the transition was applied.

    @internal
*/
bool ApplyVisualStateTransition(UIElement* element, const std::shared_ptr<VisualState>& state, bool useTransitions);
}

/*
    @summary
    Represents a named visual state with an optional storyboard.
*/
class VisualState {
public:
    explicit VisualState(std::string name = {});

    /*
        @summary
        Sets the state identifier.

        @param name
        The state name.
    */
    void SetName(std::string name) noexcept;

    /*
        @summary
        Returns the current state name.

        @returns
        The state identifier.
    */
    const std::string& GetName() const noexcept;

    /*
        @summary
        Sets the target element name for this state.

        @param targetName
        The named element to target when the state is activated.
    */
    void SetTargetName(std::string targetName) noexcept;

    /*
        @summary
        Returns the target element name for this state.

        @returns
        The optional target element name.
    */
    const std::string& GetTargetName() const noexcept;

    /*
        @summary
        Assigns a storyboard to play when the state becomes active.

        @param storyboard
        The storyboard to run for this state.
    */
    void SetStoryboard(const std::shared_ptr<NeoWPF::UI::Animation::Storyboard>& storyboard) noexcept;

    /*
        @summary
        Returns the storyboard associated with this state.

        @returns
        The storyboard for this state.
    */
    const std::shared_ptr<NeoWPF::UI::Animation::Storyboard>& GetStoryboard() const noexcept;

    /*
        @summary
        Returns whether this state has a storyboard.

        @returns
        True when a storyboard is assigned.
    */
    bool HasStoryboard() const noexcept;

private:
    std::string m_name;
    std::string m_targetName;
    std::shared_ptr<NeoWPF::UI::Animation::Storyboard> m_storyboard;
};

/*
    @summary
    Holds a set of related visual states and the group's active state.
*/
class VisualStateGroup {
public:
    explicit VisualStateGroup(std::string name = {});

    /*
        @summary
        Sets the group name.

        @param name
        The state group identifier.
    */
    void SetName(std::string name) noexcept;

    /*
        @summary
        Returns the group name.

        @returns
        The state group identifier.
    */
    const std::string& GetName() const noexcept;

    /*
        @summary
        Adds a state to the group.

        @param state
        The visual state to add.
    */
    void AddState(const std::shared_ptr<VisualState>& state);

    /*
        @summary
        Returns all states in this group.

        @returns
        The vector of visual states.
    */
    const std::vector<std::shared_ptr<VisualState>>& GetStates() const noexcept;

    /*
        @summary
        Finds a state by name.

        @param name
        The name to look for.

        @returns
        The matching state, or nullptr if none exists.
    */
    std::shared_ptr<VisualState> FindState(const std::string& name) const;

    /*
        @summary
        Returns the currently active state for the group.

        @returns
        The active visual state.
    */
    std::shared_ptr<VisualState> GetCurrentState() const noexcept;

    /*
        @summary
        Sets the currently active state.

        @param state
        The visual state to activate.
    */
    void SetCurrentState(const std::shared_ptr<VisualState>& state) noexcept;

private:
    std::string m_name;
    std::vector<std::shared_ptr<VisualState>> m_states;
    std::shared_ptr<VisualState> m_currentState;
};

/*
    @summary
    Manages visual state groups and transitions for a UI element.
*/
class VisualStateManager {
public:
    /*
        @summary
        Adds a visual state group.

        @param group
        The group to add.
    */
    void AddGroup(const std::shared_ptr<VisualStateGroup>& group);

    /*
        @summary
        Returns all registered state groups.

        @returns
        The vector of state groups.
    */
    const std::vector<std::shared_ptr<VisualStateGroup>>& GetGroups() const noexcept;

    /*
        @summary
        Finds a group by name.

        @param name
        The group identifier.

        @returns
        The matching state group, or nullptr if not found.
    */
    std::shared_ptr<VisualStateGroup> FindGroup(const std::string& name) const;

    /*
        @summary
        Finds a state inside a named group.

        @param groupName
        The group identifier.

        @param stateName
        The state identifier.

        @returns
        The matching visual state, or nullptr if not found.
    */
    std::shared_ptr<VisualState> FindState(const std::string& groupName, const std::string& stateName) const;

    /*
        @summary
        Transitions the element to a state within a specific group.

        @param element
        The UI element that owns the visual state.

        @param groupName
        The state group to search.

        @param stateName
        The target state name.

        @param useTransitions
        Whether storyboard transitions should be played.

        @returns
        True when the state was found and applied.
    */
    bool GoToState(UIElement* element, const std::string& groupName, const std::string& stateName, bool useTransitions = true);

    /*
        @summary
        Searches all groups and transitions the element to the first matching state.

        @param element
        The UI element that owns the visual state.

        @param stateName
        The target state name.

        @param useTransitions
        Whether storyboard transitions should be played.

        @returns
        True when a matching state was found and applied.
    */
    bool GoToState(UIElement* element, const std::string& stateName, bool useTransitions = true);

private:
    std::vector<std::shared_ptr<VisualStateGroup>> m_groups;
};

}
