#pragma once

#include <vector>
#include <NeoWPF/UI/Layout/Grid/Grid.h>

namespace NeoWPF::UI {

struct GridChildLayout {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

struct GridLayoutResult {
    std::vector<int> rowHeights;
    std::vector<int> colWidths;
};

struct GridLayoutMetrics {
    std::vector<int> rowHeights;
    std::vector<int> colWidths;
    std::vector<int> autoRowHeights;
    std::vector<int> autoColWidths;
    std::vector<GridChildLayout> childLayouts;
};

class GridLayout {
public:
    static GridLayoutResult ComputeLayout(
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        int availableWidth,
        int availableHeight,
        const std::vector<Visual*>& children);

    static GridLayoutMetrics ComputeMetrics(
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        int availableWidth,
        int availableHeight,
        const std::vector<Visual*>& children);

    static bool s_gridLogging;
};

}
