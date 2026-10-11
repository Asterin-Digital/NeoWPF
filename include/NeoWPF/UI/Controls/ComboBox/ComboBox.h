/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/UI/Controls/Selector/Selector.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <cstddef>
#include <memory>

namespace NeoWPF::UI {

class ListBox;

class ComboBoxItemTemplate;
class ComboBoxTemplate;

/*
    @summary
    Represents an item displayed in a ComboBox drop-down.
*/
class ComboBoxItem : public ListBoxItem {
public:
    static const Properties::DependencyProperty& IsHighlightedProperty();

    ComboBoxItem();
    ~ComboBoxItem() override;

    /*
        @summary
        Sets whether the item is highlighted.

        @param highlighted
        Highlight state.
    */
    void SetIsHighlighted(bool highlighted) noexcept;
    bool GetIsHighlighted() const noexcept;

    void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) override;
    void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) override;

private:
    void OnSelectionStateChanged() override;
    void ApplyVisualStates(bool useTransitions);
    void ApplyTheme() override;
};

class ComboBoxItemTemplate : public Templates::ControlTemplate {
public:
    ComboBoxItemTemplate();
};

class ComboBoxTemplate : public Templates::ControlTemplate {
public:
    ComboBoxTemplate();
};

/*
    @summary
    Provides a drop-down control for selecting one item.

    @details
    Items can be supplied directly or through an item source. The selected
    item is displayed when the drop-down is closed.
*/
class ComboBox : public Selector {
public:
    static const Properties::DependencyProperty& IsDropDownOpenProperty();
    static const Properties::DependencyProperty& PlacementProperty();
    static const Properties::DependencyProperty& SelectionBoxItemProperty();

    ComboBox();
    ~ComboBox() override;

    bool IsHitTestVisible() const noexcept override { return true; }
    bool IsInteractable() const noexcept override { return IsEnabled(); }

    /*
        @summary
        Opens or closes the drop-down.

        @param isOpen
        True to open the drop-down.
    */
    void SetIsDropDownOpen(bool isOpen);
    bool GetIsDropDownOpen() const noexcept;

    /*
        @summary
        Sets the drop-down placement relative to the ComboBox.

        @param placement
        Placement direction.
    */
    void SetPlacement(PopupPlacement placement);
    PopupPlacement GetPlacement() const noexcept;

    std::any GetSelectionBoxItem() const noexcept;

    /*
        @summary
        Replaces the ComboBox items.

        @param items
        Items to display in the drop-down.
    */
    void SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items);

    /*
        @summary
        Adds an item to the ComboBox.

        @param item
        Item to add.
    */
    void AddItem(std::shared_ptr<FrameworkElement> item) override;

    /*
        @summary
        Removes all ComboBox items and clears the selection.
    */
    void ClearItems();

    /*
        @summary
        Replaces the ComboBox item source.

        @param source
        Values to display in the drop-down.
    */
    void SetItemsSource(const std::vector<std::any>& source);

    /*
        @summary
        Sets the template used to create item content.

        @param templ
        Item template callback.
    */
    void SetItemTemplate(const Controls::ItemsControl::Virtualization::ItemTemplate& templ);
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

protected:
    void OnPaddingChanged(Thickness padding) override;
    void OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem) override;
private:
    void EnsurePopupTemplateParts();
    void UpdateDisplayedItem(const std::any& item);

    std::shared_ptr<Content::ContentPresenter> m_display;
    std::shared_ptr<Popup> m_popup;
    std::shared_ptr<ListBox> m_dropDown;
    std::size_t m_themeAppliedSubscriptionId = 0;
};

}
