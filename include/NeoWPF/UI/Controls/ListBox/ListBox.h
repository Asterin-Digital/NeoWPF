#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>

namespace NeoWPF::UI {

/*
    @summary
    Hosts one item in a ListBox.
*/
class ListBoxItem : public Content::ContentControl {
public:
    static const Properties::DependencyProperty& IsSelectedProperty();

    ListBoxItem();
    ~ListBoxItem() override;

    void SetIsSelected(bool selected) noexcept;
    bool GetIsSelected() const noexcept;

    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;
};

}
