#include <NeoWPF/UI/Controls/Base/GroupedToggleBase/GroupedToggleBase.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI {

GroupedToggleBase::GroupedToggleBase()
    : ToggleBase()
{
}

void GroupedToggleBase::SetChecked(bool value) noexcept
{
    if (!value || IsChecked() || m_isUpdatingGroup) {
        ToggleBase::SetChecked(value);
        return;
    }

    ToggleBase::SetChecked(true);
    UncheckGroupedPeers();
}

void GroupedToggleBase::SetGroupName(const std::wstring& groupName)
{
    if (m_groupName == groupName) {
        return;
    }

    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("GroupName", *this)) {
            SetValue(*property, groupName);
            return;
        }
    }

    m_groupName = groupName;
    InvalidateVisual();
}

bool GroupedToggleBase::IsGroupedPeer(const Visual* visual) const noexcept
{
    return dynamic_cast<const GroupedToggleBase*>(visual) != nullptr;
}

void GroupedToggleBase::OnGroupedPeerUnchecked(GroupedToggleBase* peer) noexcept
{
    (void)peer;
}

void GroupedToggleBase::UncheckGroupedPeers() noexcept
{
    if (m_isUpdatingGroup) {
        return;
    }

    Visual* scope = GetParent();
    if (!scope) {
        return;
    }

    m_isUpdatingGroup = true;
    for (Visual* visual : scope->GetVisualDescendants()) {
        if (!visual || visual == this || !IsGroupedPeer(visual)) {
            continue;
        }

        auto* peer = dynamic_cast<GroupedToggleBase*>(visual);
        if (!peer || peer->GetGroupName() != m_groupName || !peer->IsChecked()) {
            continue;
        }

        peer->SetChecked(false);
        OnGroupedPeerUnchecked(peer);
    }
    m_isUpdatingGroup = false;
}

}

