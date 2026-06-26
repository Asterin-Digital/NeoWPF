#include <NeoWPF/UI/Layout/Grid/GridLayout.h>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <NeoWPF/Diagnostics/Logger.h>
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

    NeoWPF::Diagnostics::LogInfo("[Grid] ComputeLayout: Available %dx%d, Grid %dx%d, %zu children",
        availableWidth, availableHeight, numCols, numRows, children.size());

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
            NeoWPF::Diagnostics::LogInfo("[Grid] Row %d: Fixed %d", r, def.value);
        } else if (def.mode == SizeMode::Star) {
            starRowCount += def.value > 0 ? def.value : 1;
            NeoWPF::Diagnostics::LogInfo("[Grid] Row %d: Star(%.0f)", r, (float)def.value);
        } else {
            NeoWPF::Diagnostics::LogInfo("[Grid] Row %d: Auto", r);
        }
    }

    for (int c = 0; c < numCols; ++c) {
        const auto& def = colDefs[c];
        if (def.mode == SizeMode::Fixed) {
            result.colWidths[c] = def.value;
            totalFixedWidth += def.value;
            NeoWPF::Diagnostics::LogInfo("[Grid] Col %d: Fixed %d", c, def.value);
        } else if (def.mode == SizeMode::Star) {
            starColCount += def.value > 0 ? def.value : 1;
            NeoWPF::Diagnostics::LogInfo("[Grid] Col %d: Star(%.0f)", c, (float)def.value);
        } else {
            NeoWPF::Diagnostics::LogInfo("[Grid] Col %d: Auto", c);
        }
    }

    int remainingHeight = std::max(0, availableHeight - totalFixedHeight);
    int remainingWidth = std::max(0, availableWidth - totalFixedWidth);

    int starHeightPerUnit = starRowCount > 0 ? remainingHeight / starRowCount : 0;
    int starWidthPerUnit = starColCount > 0 ? remainingWidth / starColCount : 0;

    NeoWPF::Diagnostics::LogInfo("[Grid] Star calculation: rows=%d(perUnit=%d), cols=%d(perUnit=%d)",
        starRowCount, starHeightPerUnit, starColCount, starWidthPerUnit);

    for (int r = 0; r < numRows; ++r) {
        const auto& def = rowDefs[r];
        if (def.mode == SizeMode::Star) {
            int starCount = def.value > 0 ? def.value : 1;
            result.rowHeights[r] = starHeightPerUnit * starCount;
            NeoWPF::Diagnostics::LogInfo("[Grid] Row %d (Star): %d", r, result.rowHeights[r]);
        }
    }

    for (int c = 0; c < numCols; ++c) {
        const auto& def = colDefs[c];
        if (def.mode == SizeMode::Star) {
            int starCount = def.value > 0 ? def.value : 1;
            result.colWidths[c] = starWidthPerUnit * starCount;
            NeoWPF::Diagnostics::LogInfo("[Grid] Col %d (Star): %d", c, result.colWidths[c]);
        }
    }

    int usedHeight = totalFixedHeight;
    for (int h : result.rowHeights) usedHeight += h;
    int heightRemainder = availableHeight - usedHeight;
    for (int r = 0; r < numRows && heightRemainder > 0; ++r) {
        if (rowDefs[r].mode == SizeMode::Star) {
            ++result.rowHeights[r];
            --heightRemainder;
        }
    }

    int usedWidth = totalFixedWidth;
    for (int w : result.colWidths) usedWidth += w;
    int widthRemainder = availableWidth - usedWidth;
    for (int c = 0; c < numCols && widthRemainder > 0; ++c) {
        if (colDefs[c].mode == SizeMode::Star) {
            ++result.colWidths[c];
            --widthRemainder;
        }
    }

    int totalH = 0, totalW = 0;
    for (int r = 0; r < numRows; ++r) totalH += result.rowHeights[r];
    for (int c = 0; c < numCols; ++c) totalW += result.colWidths[c];

    NeoWPF::Diagnostics::LogInfo("[Grid] Layout (Auto=0): %dx%d", totalW, totalH);

    return result;
}

}