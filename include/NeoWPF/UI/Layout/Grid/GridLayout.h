#pragma once

#include <vector>
#include <NeoWPF/UI/Layout/Grid/Grid.h>

namespace NeoWPF::UI {

struct GridLayoutResult {
    std::vector<int> rowHeights;
    std::vector<int> colWidths;
};

class GridLayout {
public:
    static GridLayoutResult ComputeLayout(
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        int availableWidth,
        int availableHeight,
        const std::vector<Visual*>& children);

    static bool s_gridLogging;
};

}
