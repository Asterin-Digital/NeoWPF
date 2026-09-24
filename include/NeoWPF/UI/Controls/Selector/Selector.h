/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <any>
#include <vector>

namespace NeoWPF::UI {

/*
    @summary
    Defines how a selector stores selected items.
*/
enum class SelectionMode {
    Single,
    Multiple,
    Extended
};

/*
    @summary
    Adds selection state to an items control.
*/
class Selector : public ItemsControl {
public:
    static const Properties::DependencyProperty& SelectedIndexProperty();
    static const Properties::DependencyProperty& SelectedItemProperty();
    static const Properties::DependencyProperty& SelectionModeProperty();

    Selector();
    ~Selector() override;

    int GetSelectedIndex() const noexcept;

    /*
        @summary
        Selects an item by index.

        @param index
        Index of the item to select.
    */
    void SetSelectedIndex(int index);

    std::any GetSelectedItem() const noexcept;

    /*
        @summary
        Selects an item by value.

        @param item
        Item to select.
    */
    void SetSelectedItem(const std::any& item);

    SelectionMode GetSelectionMode() const noexcept;

    /*
        @summary
        Sets the selection mode.

        @param mode
        Selection mode to use.
    */
    void SetSelectionMode(SelectionMode mode) noexcept;

    bool IsSelected(const std::any& item) const;

protected:
    virtual void OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem);

private:
    void UpdateSelectionState(const std::any& item);
    void SyncSelectedIndexFromState();

    std::vector<std::any> m_selectedItems;
};

}
