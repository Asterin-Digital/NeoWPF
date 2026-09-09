#pragma once

#include <memory>
#include <vector>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Controls/ItemsControl/Virtualization/ItemsControlVirtualization.h>
#include <NeoWPF/UI/Controls/ItemsControl/Virtualization/ItemTemplate.h>
#include <any>
#include <functional>
#include <unordered_map>
#include <NeoWPF/UI/Layout/Scrolling/IScrollInfo.h>
#include <NeoWPF/UI/Controls/ItemsControl/Virtualization/ItemContainerGenerator.h>

namespace NeoWPF::UI {

/*
    @summary
    A control for presenting a collection of items.

    @details
    This control stores a set of child elements and renders them in a
    vertical layout.
*/
class ItemsControl : public NeoWPF::UI::Content::ContentControl, public Layout::Scrolling::IScrollInfo {
public:
    using ItemCollection = std::vector<std::shared_ptr<FrameworkElement>>;

    static const Properties::DependencyProperty& ItemsProperty();

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
    virtual void AddItem(std::shared_ptr<FrameworkElement> item);

    /*
        @summary
        Removes all items from the collection.
    */
    void ClearItems();

    /*
        @summary
        Sets an external items source for the control.

        @param source
        A vector of values (`std::any`) that represent the data items. When
        using an `ItemsSource`, the control will materialize visuals via the
        configured item template or the generator.
    */
    void SetItemsSource(const std::vector<std::any>& source);

    /*
        @summary
        Sets the factory used to create item visuals from an `ItemsSource` item.

        @param templ
        A callable that produces a `FrameworkElement` given a source item and
        its index.
    */
    void SetItemTemplate(const Controls::ItemsControl::Virtualization::ItemTemplate& templ);

    /*
        @summary
        Requests a custom item container for a source item.

        @param data
        The source object for the item.

        @param index
        The index of the item in the source collection.

        @returns A custom container instance, or nullptr to use the item template.
    */
    virtual std::shared_ptr<FrameworkElement> GetContainerForItem(const std::any& data, int index);

    /*
        @summary
        Prepares a container for display using the source item data.

        @param container
        The container to prepare.

        @param data
        The source object for the item.

        @param index
        The index of the item in the source collection.
    */
    virtual void PrepareContainer(const std::shared_ptr<FrameworkElement>& container,
                                  const std::any& data,
                                  int index);

    /*
        @summary
        Returns the current collection of items.
    */
    const std::vector<std::shared_ptr<FrameworkElement>>& GetItems() const noexcept { return m_items; }
    const ItemCollection& GetItemsValue() const noexcept { return m_items; }
    void SetItemsValue(const ItemCollection& items) { SetItems(items); }

    int GetItemCount() const noexcept { return static_cast<int>(m_hasItemsSource ? m_itemsSource.size() : m_items.size()); }

    /*
        @summary
        Sets the spacing between items.

        @param spacing
        The spacing in pixels.
    */
    void SetSpacing(int spacing) noexcept {
        SetVerticalSpacing(spacing);
        InvalidateMeasure();
        InvalidateArrange();
        InvalidateVisual();
    }

    /*
        @summary
        Returns the spacing between items.
    */
    int GetSpacing() const noexcept { return GetVerticalSpacing(); }

    /*
        @summary
        Sets the padding around the item area.

        @param padding
        The padding in pixels.
    */
    void SetPadding(Thickness padding) noexcept {
        m_padding = padding.Left;
        InvalidateMeasure();
        InvalidateArrange();
        InvalidateVisual();
    }

    /*
        @summary
        Returns the padding around the item area.
    */
    Thickness GetPadding() const noexcept { return Thickness(m_padding); }

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

    /* IScrollInfo: extent in device pixels */
    int GetExtentWidth() const noexcept override;
    int GetExtentHeight() const noexcept override;

    /* IScrollInfo: viewport dimensions */
    int GetViewportWidth() const noexcept override;
    int GetViewportHeight() const noexcept override;

    /* IScrollInfo: current scroll offsets */
    int GetHorizontalOffset() const noexcept override;
    int GetVerticalOffset() const noexcept override;

    /* IScrollInfo: set scroll offsets */
    void SetHorizontalOffset(int offset) noexcept override;
    void SetVerticalOffset(int offset) noexcept override;

    /* IScrollInfo: small-line scrolling commands */
    void LineLeft() noexcept override;
    void LineRight() noexcept override;
    void LineUp() noexcept override;
    void LineDown() noexcept override;

    /* IScrollInfo: page scrolling commands */
    void PageLeft() noexcept override;
    void PageRight() noexcept override;
    void PageUp() noexcept override;
    void PageDown() noexcept override;

    /* IScrollInfo: mouse wheel scroll delta */
    void MouseWheel(int delta) noexcept override;

private:
    std::vector<std::shared_ptr<FrameworkElement>> m_items;
    int m_padding = 0;
    std::shared_ptr<Controls::ItemsControl::Virtualization::ItemsControlVirtualizer> m_virtualization;
    std::vector<std::any> m_itemsSource;
    bool m_hasItemsSource = false;
    Controls::ItemsControl::Virtualization::ItemTemplate m_itemTemplate;
    std::unordered_map<int, std::shared_ptr<FrameworkElement>> m_realizedItems;
    std::shared_ptr<Controls::ItemsControl::Virtualization::ItemContainerGenerator> m_generator;
    void ClearRealizedItems() noexcept;
    void RealizeSourceItems();
    /*
        @summary
        Ensure visual containers exist for the half-open index range [first, last].

        @param first
        First index to realize (inclusive).
        @param last
        Last index to realize (inclusive).
    */
    void RealizeRange(int first, int last);
public:
    /*
        @summary
        Access the item container generator used to acquire/recycle item visuals.
    */
    std::shared_ptr<Controls::ItemsControl::Virtualization::ItemContainerGenerator> GetGenerator() const noexcept { return m_generator; }

    /*
        @summary
        Returns the configured item template.
    */
    const Controls::ItemsControl::Virtualization::ItemTemplate& GetItemTemplate() const noexcept { return m_itemTemplate; }

    /*
        @summary
        Returns the current items source vector (may be empty for inline items).
    */
    const std::vector<std::any>& GetItemsSource() const noexcept { return m_itemsSource; }

    /*
        @summary
        Assign a custom virtualization engine instance.
    */
    void SetVirtualization(const std::shared_ptr<Controls::ItemsControl::Virtualization::ItemsControlVirtualizer>& virtualization) noexcept { m_virtualization = virtualization; }

    /*
        @summary
        Retrieve the assigned virtualization engine, or `nullptr` if none.
    */
    std::shared_ptr<Controls::ItemsControl::Virtualization::ItemsControlVirtualizer> GetVirtualization() const noexcept { return m_virtualization; }

    /*
        @summary
        Toggle a default virtualization engine on or off.

        @param enabled
        `true` creates and assigns a default engine; `false` removes any
        existing engine and the control will use non-virtualized behavior.
    */
    void SetVirtualization(bool enabled) noexcept;
};

}
