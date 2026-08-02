#pragma once

#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <initializer_list>
#include <vector>
#include <unordered_map>

namespace NeoWPF::UI {


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
        Holds sizing information for a grid row or column.
    */
    struct Definition {
        SizeMode mode = SizeMode::Auto;
        int value = 0;
    };

    Grid() = default;

    /*
        @summary
        Adds a row definition to the grid.

        @param mode
        Controls row sizing: Auto (fits content), Fixed (pixel value), or Star (distributes remaining space).

        @param value
        The pixel size for Fixed rows or the weight for Star rows. Auto rows ignore this.
    */
    void AddRow(SizeMode mode = SizeMode::Auto, int value = 0) { m_rowDefs.push_back({mode, value}); }

    /*
        @summary
        Adds several row definitions at once.

        @param defs
        The rows to add, in order.
    */
    void AddRows(std::initializer_list<Definition> defs) { m_rowDefs.insert(m_rowDefs.end(), defs.begin(), defs.end()); }

    /*
        @summary
        Adds a column definition to the grid.

        @param mode
        Whether the column should size to its contents, use a fixed pixel value, or share leftover space.

        @param value
        The pixel size for Fixed columns or the weight for Star columns. Auto columns ignore this.
    */
    void AddColumn(SizeMode mode = SizeMode::Auto, int value = 0) { m_colDefs.push_back({mode, value}); }

    /*
        @summary
        Adds several column definitions at once.

        @param defs
        The columns to add, in order.
    */
    void AddColumns(std::initializer_list<Definition> defs) { m_colDefs.insert(m_colDefs.end(), defs.begin(), defs.end()); }

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
    */
    static void SetRow(Visual* child, int row);

    /*
        @summary
        Sets the column for a child.

        @param child
        The child to update.

        @param column
        The column index.
    */
    static void SetColumn(Visual* child, int column);

    /*
        @summary
        Gets the row assigned to a child.

        @param child
        The child to inspect.
    */
    static int GetRow(Visual* child);

    /*
        @summary
        Gets the column assigned to a child.

        @param child
        The child to inspect.
    */
    static int GetColumn(Visual* child);

    /*
        @summary
        Clears any Grid attached properties for a child.

        @param child
        The child whose Grid attached state should be removed.

        @internal
    */
    static void ClearAttachedProperties(Visual* child);

    /*
        @summary
        Sets how many rows the child should span.

        @param child
        The child to update.

        @param span
        The number of rows the child should cover.
    */
    static void SetRowSpan(Visual* child, int span);

    /*
        @summary
        Sets how many columns the child should span.

        @param child
        The child to update.

        @param span
        The number of columns the child should cover.
    */
    static void SetColumnSpan(Visual* child, int span);

    /*
        @summary
        Gets the row span for a child.

        @param child
        The child to inspect.
    */
    static int GetRowSpan(Visual* child);

    /*
        @summary
        Gets the column span for a child.

        @param child
        The child to inspect.
    */
    static int GetColumnSpan(Visual* child);

    /*
        @summary
        Measures the grid and resolves Auto-sized rows and columns.

        @param availableWidth
        The width available to the grid.

        @param availableHeight
        The height available to the grid.
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Positions each child inside its assigned cell.

        @param finalWidth
        The width assigned to the grid.

        @param finalHeight
        The height assigned to the grid.
    */
    void Arrange(int finalWidth, int finalHeight) override;

protected:
    void OnRemoveVisualChild(Visual* child) override;

private:
    std::vector<Definition> m_rowDefs;
    std::vector<Definition> m_colDefs;
    std::vector<int> m_autoRowHeights;
    std::vector<int> m_autoColWidths;

    static std::unordered_map<Visual*, int> s_rowMap;
    static std::unordered_map<Visual*, int> s_colMap;
    static std::unordered_map<Visual*, int> s_rowSpanMap;
    static std::unordered_map<Visual*, int> s_colSpanMap;
};

}
