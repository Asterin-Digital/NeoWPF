/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>

namespace NeoWPF::UI {

/*
    @summary
    Provides mutually exclusive checked state for grouped toggle controls.

    @details
    Grouped controls use the visual tree as their group scope. Derived controls
    can override IsGroupedPeer to restrict which controls participate in a group.
*/
class GroupedToggleBase : public ToggleBase {
public:
    static const Properties::DependencyProperty& GroupNameProperty();

    GroupedToggleBase();
    ~GroupedToggleBase() override = default;

    void SetChecked(bool value) noexcept override;

    void SetGroupName(const std::wstring& groupName);
    const std::wstring& GetGroupName() const noexcept { return m_groupName; }

protected:
    virtual bool IsGroupedPeer(const Visual* visual) const noexcept;
    virtual void OnGroupedPeerUnchecked(GroupedToggleBase* peer) noexcept;

private:
    void UncheckGroupedPeers() noexcept;

    std::wstring m_groupName;
    bool m_isUpdatingGroup = false;
};

}