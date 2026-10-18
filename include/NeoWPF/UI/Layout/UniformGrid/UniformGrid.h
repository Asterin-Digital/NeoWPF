/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Layout/Panel/Panel.h>

namespace NeoWPF::UI {

/*
    @summary
    Lays out children in a grid of equal-sized cells.

    @details
    Rows and Columns default to zero for automatic sizing; negative values are also
    automatic and report an error. FirstColumn defaults to zero; out-of-range values
    are clamped with an error. Insufficient explicit capacity adds effective rows with
    a warning.

    @example
    auto grid = std::make_shared<NeoWPF::UI::UniformGrid>();
    grid->SetRows(2);
    grid->SetColumns(3);
    grid->SetFirstColumn(1);
    grid->AddChild(std::make_shared<NeoWPF::UI::TextBlock>(L"First"));
*/
class UniformGrid : public Panel {
public:
    UniformGrid();

    static const NeoWPF::UI::Properties::DependencyProperty& RowsProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& ColumnsProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& FirstColumnProperty();

    /*
        @summary
        Gets the configured row count.

        @returns
        The row count, or zero when rows are automatic.
    */
    int GetRows() const;

    /*
        @summary
        Sets the requested row count.

        @param rows
        The row count, or zero to choose it automatically.
    */
    void SetRows(int rows);

    /*
        @summary
        Gets the configured column count.

        @returns
        The column count, or zero when columns are automatic.
    */
    int GetColumns() const;

    /*
        @summary
        Sets the requested column count.

        @param columns
        The column count, or zero to choose it automatically.
    */
    void SetColumns(int columns);

    /*
        @summary
        Gets the requested starting column for the first child.

        @returns
        The configured first column.
    */
    int GetFirstColumn() const;

    /*
        @summary
        Sets the starting column for the first child.

        @param firstColumn
        The column at which the first visible child is placed.
    */
    void SetFirstColumn(int firstColumn);

    /*
        @summary
        Measures the grid and its visible children.

        @param availableWidth
        The width available to the grid.

        @param availableHeight
        The height available to the grid.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Arranges visible children into equal-sized cells.

        @param finalWidth
        The grid's final width.

        @param finalHeight
        The grid's final height.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;
};

}