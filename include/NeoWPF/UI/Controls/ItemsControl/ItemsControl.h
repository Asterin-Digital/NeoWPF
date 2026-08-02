#pragma once

#include <memory>
#include <vector>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

namespace NeoWPF::UI {

/*
    @summary
    A control for presenting a collection of items.

    @details
    This control stores a set of child elements and renders them in a
    vertical layout.
*/
class ItemsControl : public NeoWPF::UI::Content::ContentControl {
public:
    ItemsControl();

    ~ItemsControl();

    /*
        @summary
        Replaces the current item collection.

        @param items
        The items to display.
    */
    void SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items);

    /*
        @summary
        Appends a single item to the collection.

        @param item
        The item to add.
    */
    void AddItem(const std::shared_ptr<FrameworkElement>& item);

    /*
        @summary
        Removes all items from the collection.
    */
    void ClearItems();

    /*
        @summary
        Returns the current collection of items.
    */
    const std::vector<std::shared_ptr<FrameworkElement>>& GetItems() const noexcept { return m_items; }

    /*
        @summary
        Sets the spacing between items.

        @param spacing
        The spacing in pixels.
    */
    void SetItemSpacing(int spacing) noexcept { m_itemSpacing = spacing; InvalidateVisual(); }

    /*
        @summary
        Returns the spacing between items.
    */
    int GetItemSpacing() const noexcept { return m_itemSpacing; }

    /*
        @summary
        Sets the padding around the item area.

        @param padding
        The padding in pixels.
    */
    void SetPadding(int padding) noexcept { m_padding = padding; InvalidateVisual(); }

    /*
        @summary
        Returns the padding around the item area.
    */
    int GetPadding() const noexcept { return m_padding; }

    /*
        @summary
        Returns whether the control participates in hit testing.
    */
    bool IsHitTestVisible() const noexcept override { return false; }

    /*
        @summary
        Measures the control and determines the space it needs.

        @param availableWidth
        Width available for layout.

        @param availableHeight
        Height available for layout.

        @internal
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Positions the control and its items within the allocated area.

        @param finalWidth
        The width assigned to the control.

        @param finalHeight
        The height assigned to the control.

        @internal
    */
    void Arrange(int finalWidth, int finalHeight) override;

    /*
        @summary
        Builds the render commands used to draw the control.

        @param commands
        The command list that receives the rendering instructions.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

private:
    std::vector<std::shared_ptr<FrameworkElement>> m_items;
    int m_itemSpacing = 4;
    int m_padding = 0;
};

}
