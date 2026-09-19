/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <vector>
#include <NeoWPF/UI/Layout/Grid/Grid.h>

namespace NeoWPF::UI {

struct GridChildLayout {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

struct GridLayoutResult {
    std::vector<double> rowHeights;
    std::vector<double> colWidths;
};

struct GridLayoutMetrics {
    std::vector<double> rowHeights;
    std::vector<double> colWidths;
    std::vector<double> autoRowHeights;
    std::vector<double> autoColWidths;
    std::vector<GridChildLayout> childLayouts;
};

class GridLayout {
public:
    static std::vector<Grid::Definition> ResolveDefinitions(const std::vector<Grid::Definition>& defs);

    static GridLayoutResult ComputeLayout(
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        double availableWidth,
        double availableHeight,
        const std::vector<Visual*>& children,
        double horizontalSpacing = 0.0,
        double verticalSpacing = 0.0);

    static GridLayoutMetrics ComputeMetrics(
        const Grid* grid,
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        double availableWidth,
        double availableHeight,
        const std::vector<Visual*>& children,
        double horizontalSpacing = 0.0,
        double verticalSpacing = 0.0);
};

}
