#include <NeoWPF/UI/Layout/Grid/GridLayout.h>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <algorithm>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace NeoWPF::UI {

GridLayoutResult GridLayout::ComputeLayout(
    const std::vector<Grid::Definition>& rowDefs,
    const std::vector<Grid::Definition>& colDefs,
    int availableWidth,
    int availableHeight,
    const std::vector<Visual*>& children)
{
    int numRows = std::max(1, static_cast<int>(rowDefs.size()));
    int numCols = std::max(1, static_cast<int>(colDefs.size()));

    GridLayoutResult result;
    result.rowHeights.resize(numRows, 0);
    result.colWidths.resize(numCols, 0);

    int totalFixedHeight = 0;
    int totalFixedWidth = 0;
    int starRowCount = 0;
    int starColCount = 0;

    for (int r = 0; r < numRows; ++r) {
        const auto& def = rowDefs[r];
        if (def.mode == SizeMode::Fixed) {
            result.rowHeights[r] = def.value;
            totalFixedHeight += def.value;
        } else if (def.mode == SizeMode::Star) {
            starRowCount += def.value > 0 ? def.value : 1;
        }
    }

    for (int c = 0; c < numCols; ++c) {
        const auto& def = colDefs[c];
        if (def.mode == SizeMode::Fixed) {
            result.colWidths[c] = def.value;
            totalFixedWidth += def.value;
        } else if (def.mode == SizeMode::Star) {
            starColCount += def.value > 0 ? def.value : 1;
        }
    }

    int remainingHeight = std::max(0, availableHeight - totalFixedHeight);
    int remainingWidth = std::max(0, availableWidth - totalFixedWidth);

    int starHeightPerUnit = starRowCount > 0 ? remainingHeight / starRowCount : 0;
    int starWidthPerUnit = starColCount > 0 ? remainingWidth / starColCount : 0;

    for (int r = 0; r < numRows; ++r) {
        const auto& def = rowDefs[r];
        if (def.mode == SizeMode::Star) {
            int starCount = def.value > 0 ? def.value : 1;
            result.rowHeights[r] = starHeightPerUnit * starCount;
        } else if (def.mode == SizeMode::Auto) {
            int maxAutoHeight = 0;
            for (const auto& child : children) {
                if (!child) continue;
                if (Grid::GetRow(child) == r) {
                    if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
                        maxAutoHeight = std::max(maxAutoHeight, fe->GetDesiredHeight());
                    }
                }
            }
            result.rowHeights[r] = maxAutoHeight;
            totalFixedHeight += maxAutoHeight;
        }
    }

    for (int c = 0; c < numCols; ++c) {
        const auto& def = colDefs[c];
        if (def.mode == SizeMode::Star) {
            int starCount = def.value > 0 ? def.value : 1;
            result.colWidths[c] = starWidthPerUnit * starCount;
        } else if (def.mode == SizeMode::Auto) {
            int maxAutoWidth = 0;
            for (const auto& child : children) {
                if (!child) continue;
                if (Grid::GetColumn(child) == c) {
                    if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
                        maxAutoWidth = std::max(maxAutoWidth, fe->GetDesiredWidth());
                    }
                }
            }
            result.colWidths[c] = maxAutoWidth;
            totalFixedWidth += maxAutoWidth;
        }
    }

    return result;
}

}
