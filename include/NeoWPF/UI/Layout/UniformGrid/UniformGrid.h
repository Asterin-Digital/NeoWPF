/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Layout/Panel/Panel.h>

namespace NeoWPF::UI {

/*
    @summary
    Arranges visible framework-element children in equal-sized cells.

    @details
    UniformGrid places children in row-major order. Rows and Columns default to zero,
    which requests automatic sizing. When both are automatic, the grid chooses a
    near-square layout. FirstColumn offsets the first participating child. Negative
    dimensions are treated as automatic and invalid FirstColumn values are clamped;
    invalid values and explicit-capacity overflow are reported through developer
    diagnostics. Invisible children do not consume cells. Cells have no gaps, and
    inherited Padding and Spacing do not affect their geometry.
*/
class UniformGrid : public Panel {
public:
    UniformGrid();

    static const NeoWPF::UI::Properties::DependencyProperty& RowsProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& ColumnsProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& FirstColumnProperty();

    int GetRows() const;
    void SetRows(int rows);
    int GetColumns() const;
    void SetColumns(int columns);
    int GetFirstColumn() const;
    void SetFirstColumn(int firstColumn);

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
};

}