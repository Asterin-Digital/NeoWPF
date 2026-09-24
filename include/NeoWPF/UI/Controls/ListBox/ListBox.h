/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Controls/Selector/Selector.h>

namespace NeoWPF::UI {

/*
    @summary
    Displays a selectable list of items.
*/
class ListBox : public Selector {
public:
    ListBox();
    ~ListBox() override;

    /*
        @summary
        Adds an item to the list.

        @param item
        Item to add.
    */
    void AddItem(std::shared_ptr<FrameworkElement> item) override;

    /*
        @summary
        Replaces the list items.

        @param items
        Items to display.
    */
    void SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items);

    /*
        @summary
        Synchronizes the list selection with an item value.

        @param item
        Item to select.
    */
    virtual void SyncSelection(const std::any& item);

    std::shared_ptr<FrameworkElement> GetContainerForItem(const std::any& data, int index) override;
    void PrepareContainer(const std::shared_ptr<FrameworkElement>& container,
                          const std::any& data,
                          int index) override;

    void OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem) override;

private:
    void AttachSelectionHandler(const std::shared_ptr<FrameworkElement>& item);
};

/*
    @summary
    Hosts one item in a ListBox.
*/
class ListBoxItem : public Content::ContentControl {
public:
    static const Properties::DependencyProperty& IsSelectedProperty();

    ListBoxItem();
    ~ListBoxItem() override;

    /*
        @summary
        Associates the item with its owning list.

        @param owner
        Owning list.
    */
    void SetOwner(ListBox* owner) noexcept;
    ListBox* GetOwner() const noexcept;

    /*
        @summary
        Sets the selected state of the item.

        @param selected
        Selection state.
    */
    void SetIsSelected(bool selected) noexcept;
    bool GetIsSelected() const noexcept;

    void OnMouseUp(int x, int y, std::uint32_t buttonState) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

protected:
    virtual void OnSelectionStateChanged() {}

private:
    ListBox* m_owner = nullptr;
};

}
