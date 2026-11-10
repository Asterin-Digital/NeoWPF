/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <initializer_list>
#include <vector>
#include <unordered_map>
#include <memory>

namespace NeoWPF::UI {

class RowDefinition;
class ColumnDefinition;
class Grid;

/*
    @summary
    Tracks a Grid's lifetime while definitions notify their owners.

    @internal
*/
struct GridDefinitionOwner {
    Grid* owner = nullptr;
};


/*
    @summary
    Defines how a row or column should size itself.

    @details
    Auto lets the row or column fit its contents, Fixed uses the value you give it, and Star takes the remaining space.
*/
enum class SizeMode {
    Auto,
    Fixed,
    Star
};

/*
    @summary
    Lays out children in a grid of rows and columns.

    @details
    Rows and columns must be added before children can be placed. Row and column indices are zero-based
    Tip: don't set explicit w/h on grid, it will not scale to new window size

    @example
    auto grid = std::make_shared<NeoWPF::UI::Grid>();
    auto button = std::make_shared<NeoWPF::Button>(L"OK");
    grid->AddRow(NeoWPF::UI::SizeMode::Auto, 0);
    grid->AddColumn(NeoWPF::UI::SizeMode::Star, 1);
    grid->AddChild(button, 0, 0);
*/
class Grid : public Panel {
public:
    using Panel::AddChild;

    /*
        @summary
        Stores the sizing mode and value for a grid row or column.
    */
    struct Definition {
        SizeMode mode = SizeMode::Auto;
        int value = 0;
    };

    /*
        @summary
        Represents a fixed, content-sized, or weighted grid dimension.
    */
    struct GridLength {
        SizeMode Mode = SizeMode::Auto;
        int Value = 0;

        /*
            @summary
            Creates a content-sized grid dimension.
        */
        static GridLength Auto() noexcept { return {SizeMode::Auto, 0}; }
        /*
            @summary
            Creates a star-sized grid dimension with the supplied weight.

            @param weight
            Relative sizing weight.
        */
        static GridLength Star(int weight = 1) noexcept { return {SizeMode::Star, weight > 0 ? weight : 1}; }
        /*
            @summary
            Creates a fixed grid dimension with the supplied pixel size.

            @param pixels
            Pixel size to use.
        */
        static GridLength Fixed(int pixels) noexcept { return {SizeMode::Fixed, pixels > 0 ? pixels : 0}; }
    };

    Grid() : m_definitionOwner(std::make_shared<GridDefinitionOwner>(GridDefinitionOwner{this})) {
    }

    ~Grid() override;

    static const NeoWPF::UI::Properties::DependencyProperty& RowProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& ColumnProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& RowSpanProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& ColumnSpanProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& RowDefinitionsProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& ColumnDefinitionsProperty();

    /*
        @summary
        Adds a row definition to the grid.

        @param mode
        Controls row sizing: Auto (fits content), Fixed (pixel value), or Star (distributes remaining space).

        @param value
        The pixel size for Fixed rows or the weight for Star rows. Auto rows ignore this.
    */
    void AddRow(SizeMode mode = SizeMode::Auto, int value = 0);

    /*
        @summary
        Adds several row definitions at once.

        @param defs
        The rows to add, in order.
    */
    void AddRows(std::initializer_list<Definition> defs);

    /*
        @summary
        Adds a column definition to the grid.

        @param mode
        Whether the column should size to its contents, use a fixed pixel value, or share leftover space.

        @param value
        The pixel size for Fixed columns or the weight for Star columns. Auto columns ignore this.
    */
    void AddColumn(SizeMode mode = SizeMode::Auto, int value = 0);

    /*
        @summary
        Adds several column definitions at once.

        @param defs
        The columns to add, in order.
    */
    void AddColumns(std::initializer_list<Definition> defs);

    /*
        @summary
        Adds a RowDefinition to the grid.

        @param rowDef
        The RowDefinition object to add.

        @remarks
        This is used by XAML parsing to add row definitions created from XAML tags.
    */
    void AddRowDefinition(std::shared_ptr<RowDefinition> rowDef);

    /*
        @summary
        Adds a ColumnDefinition to the grid.

        @param colDef
        The ColumnDefinition object to add.

        @remarks
        This is used by XAML parsing to add column definitions created from XAML tags.
    */
    void AddColumnDefinition(std::shared_ptr<ColumnDefinition> colDef);

    /*
        @summary
        Returns the row definitions assigned to the grid.
    */
    const std::vector<Definition>& GetRowDefinitions() const noexcept { return m_rowDefs; }
    /*
        @summary
        Returns the column definitions assigned to the grid.
    */
    const std::vector<Definition>& GetColumnDefinitions() const noexcept { return m_colDefs; }

    /*
        @summary
        Places a child in a grid cell.

        @param child
        The child to place.

        @param row
        The row index.

        @param column
        The column index.

        @param rowSpan
        How many rows the child should cover.

        @param columnSpan
        How many columns the child should cover.
    */
    void AddChild(const std::shared_ptr<Visual>& child, int row, int column, int rowSpan = 1, int columnSpan = 1);

    /*
        @summary
        Sets the row for a child.

        @param child
        The child to update.

        @param row
        The row index.

        @internal
    */
    void SetRow(Visual* child, int row) noexcept;

    /*
        @summary
        Sets the column for a child.

        @param child
        The child to update.

        @param column
        The column index.

        @internal
    */
    void SetColumn(Visual* child, int column) noexcept;

    /*
        @summary
        Gets the row assigned to a child.

        @param child
        The child to inspect.

        @internal
    */
    int GetRow(Visual* child) const noexcept;

    /*
        @summary
        Gets the column assigned to a child.

        @param child
        The child to inspect.

        @internal
    */
    int GetColumn(Visual* child) const noexcept;

    /*
        @summary
        Clears any Grid attached properties for a child.

        @param child
        The child whose Grid attached state should be removed.

        @internal
    */
    void ClearAttachedProperties(Visual* child);

    /*
        @summary
        Sets how many rows the child should span.

        @param child
        The child to update.

        @param span
        The number of rows the child should cover.

        @internal
    */
    static void SetRowSpan(Visual* child, int span);

    /*
        @summary
        Sets how many columns the child should span.

        @param child
        The child to update.

        @param span
        The number of columns the child should cover.

        @internal
    */
    static void SetColumnSpan(Visual* child, int span);

    /*
        @summary
        Gets the row span for a child.

        @param child
        The child to inspect.

        @internal
    */
    static int GetRowSpan(Visual* child);

    /*
        @summary
        Gets the column span for a child.

        @param child
        The child to inspect.

        @internal
    */
    static int GetColumnSpan(Visual* child);

    /*
        @summary
        Measures the grid and resolves Auto-sized rows and columns.

        @param availableWidth
        The width available to the grid.

        @param availableHeight
        The height available to the grid.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Positions each child inside its assigned cell.

        @param finalWidth
        The width assigned to the grid.

        @param finalHeight
        The height assigned to the grid.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

protected:
    /*
        @summary
        Called after a child visual has been removed.

        @internal
    */
    void OnRemoveVisualChild(Visual* child) override;

private:
    friend class RowDefinition;
    friend class ColumnDefinition;

    /*
        @summary
        Updates the cached row definition after its value changes.

        @internal
    */
    void OnRowDefinitionChanged(RowDefinition* definition);

    /*
        @summary
        Updates the cached column definition after its value changes.

        @internal
    */
    void OnColumnDefinitionChanged(ColumnDefinition* definition);

    std::shared_ptr<GridDefinitionOwner> m_definitionOwner;
    std::vector<Definition> m_rowDefs;
    std::vector<Definition> m_colDefs;
    std::vector<std::shared_ptr<RowDefinition>> m_rowDefinitionObjects;
    std::vector<std::shared_ptr<ColumnDefinition>> m_columnDefinitionObjects;
    std::vector<double> m_autoRowHeights;
    std::vector<double> m_autoColWidths;
};

}
