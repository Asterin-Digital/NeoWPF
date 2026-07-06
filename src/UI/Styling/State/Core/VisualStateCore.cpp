#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

#include <algorithm>

namespace NeoWPF::UI::Styling {

VisualState::VisualState(std::string name)
    : m_name(std::move(name))
{
}

void VisualState::SetName(std::string name) noexcept {
    m_name = std::move(name);
}

const std::string& VisualState::GetName() const noexcept {
    return m_name;
}

void VisualState::SetTargetName(std::string targetName) noexcept {
    m_targetName = std::move(targetName);
}

const std::string& VisualState::GetTargetName() const noexcept {
    return m_targetName;
}

void VisualState::SetStoryboard(const std::shared_ptr<NeoWPF::UI::Animation::Storyboard>& storyboard) noexcept {
    m_storyboard = storyboard;
}

const std::shared_ptr<NeoWPF::UI::Animation::Storyboard>& VisualState::GetStoryboard() const noexcept {
    return m_storyboard;
}

bool VisualState::HasStoryboard() const noexcept {
    return static_cast<bool>(m_storyboard);
}

VisualStateGroup::VisualStateGroup(std::string name)
    : m_name(std::move(name))
{
}

void VisualStateGroup::SetName(std::string name) noexcept {
    m_name = std::move(name);
}

const std::string& VisualStateGroup::GetName() const noexcept {
    return m_name;
}

void VisualStateGroup::AddState(const std::shared_ptr<VisualState>& state) {
    if (!state) {
        return;
    }

    if (std::find_if(m_states.begin(), m_states.end(), [&](const auto& candidate) {
            return candidate && candidate->GetName() == state->GetName();
        }) != m_states.end()) {
        return;
    }

    m_states.push_back(state);
}

const std::vector<std::shared_ptr<VisualState>>& VisualStateGroup::GetStates() const noexcept {
    return m_states;
}

std::shared_ptr<VisualState> VisualStateGroup::FindState(const std::string& name) const {
    if (name.empty()) {
        return nullptr;
    }

    auto it = std::find_if(m_states.begin(), m_states.end(), [&](const auto& state) {
        return state && state->GetName() == name;
    });

    return it != m_states.end() ? *it : nullptr;
}

std::shared_ptr<VisualState> VisualStateGroup::GetCurrentState() const noexcept {
    return m_currentState;
}

void VisualStateGroup::SetCurrentState(const std::shared_ptr<VisualState>& state) noexcept {
    m_currentState = state;
}

}
