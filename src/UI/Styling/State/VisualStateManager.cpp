#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

#include <algorithm>

namespace NeoWPF::UI::Styling {

void VisualStateManager::AddGroup(const std::shared_ptr<VisualStateGroup>& group) {
    if (!group) {
        return;
    }

    if (std::find_if(m_groups.begin(), m_groups.end(), [&](const auto& candidate) {
            return candidate && candidate->GetName() == group->GetName();
        }) != m_groups.end()) {
        return;
    }

    m_groups.push_back(group);
}

const std::vector<std::shared_ptr<VisualStateGroup>>& VisualStateManager::GetGroups() const noexcept {
    return m_groups;
}

std::shared_ptr<VisualStateGroup> VisualStateManager::FindGroup(const std::string& name) const {
    if (name.empty()) {
        return nullptr;
    }

    auto it = std::find_if(m_groups.begin(), m_groups.end(), [&](const auto& group) {
        return group && group->GetName() == name;
    });

    return it != m_groups.end() ? *it : nullptr;
}

std::shared_ptr<VisualState> VisualStateManager::FindState(const std::string& groupName, const std::string& stateName) const {
    if (groupName.empty() || stateName.empty()) {
        return nullptr;
    }

    if (auto group = FindGroup(groupName)) {
        return group->FindState(stateName);
    }

    return nullptr;
}

bool VisualStateManager::GoToState(UIElement* element, const std::string& groupName, const std::string& stateName, bool useTransitions) {
    if (!element || groupName.empty() || stateName.empty()) {
        return false;
    }

    if (auto group = FindGroup(groupName)) {
        if (auto target = group->FindState(stateName)) {
            if (target == group->GetCurrentState()) {
                return true;
            }

            group->SetCurrentState(target);
            return detail::ApplyVisualStateTransition(element, target, useTransitions);
        }
    }

    return false;
}

bool VisualStateManager::GoToState(UIElement* element, const std::string& stateName, bool useTransitions) {
    if (!element || stateName.empty()) {
        return false;
    }

    for (const auto& group : m_groups) {
        if (!group) {
            continue;
        }

        if (auto target = group->FindState(stateName)) {
            group->SetCurrentState(target);
            return detail::ApplyVisualStateTransition(element, target, useTransitions);
        }
    }

    return false;
}

}
