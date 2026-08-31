#include <NeoWPF/UI/Layout/Grid/GridLayout.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace NeoWPF::UI {

    namespace {
        std::vector<Grid::Definition> NormalizeDefinitions(
            const std::vector<Grid::Definition>& defs,
            const std::vector<int>& autoSizes)
        {
            std::vector<Grid::Definition> normalized = GridLayout::ResolveDefinitions(defs);

            for (size_t i = 0; i < normalized.size(); ++i) {
                if (normalized[i].mode == SizeMode::Auto) {
                    normalized[i].mode = SizeMode::Fixed;
                    normalized[i].value = i < autoSizes.size() ? autoSizes[i] : 0;
                }
            }

            return normalized;
        }

        std::vector<int> ResolveAutoSizes(
            const Grid* grid,
            const std::vector<Grid::Definition>& defs,
            const std::vector<Visual*>& children,
            bool resolveRowHeights)
        {
            const auto effectiveDefs = GridLayout::ResolveDefinitions(defs);
            const int count = std::max(1, static_cast<int>(effectiveDefs.size()));
            std::vector<int> autoSizes(count, 0);

            for (int index = 0; index < count; ++index) {
                const bool isAuto = effectiveDefs[index].mode == SizeMode::Auto || effectiveDefs[index].mode == SizeMode::Star;
                if (!isAuto) {
                    continue;
                }

                int maxSize = 0;
                for (auto child : children) {
                    if (!child || !grid) {
                        continue;
                    }

                    if (resolveRowHeights) {
                        if (grid->GetRow(child) == index && Grid::GetRowSpan(child) == 1) {
                            if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
                                const auto margin = fe->GetMargin();
                                maxSize = std::max(maxSize, fe->GetDesiredHeight() + margin.Vertical());
                            }
                        }
                    }
                    else {
                        if (grid->GetColumn(child) == index && Grid::GetColumnSpan(child) == 1) {
                            if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
                                const auto margin = fe->GetMargin();
                                maxSize = std::max(maxSize, fe->GetDesiredWidth() + margin.Horizontal());
                            }
                        }
                    }
                }

                autoSizes[index] = maxSize;
            }

            return autoSizes;
        }
    }


    std::vector<Grid::Definition> GridLayout::ResolveDefinitions(const std::vector<Grid::Definition>& defs)
    {
        if (defs.empty()) {
            return { { SizeMode::Star, 1 } };
        }

        return defs;
    }

    GridLayoutResult GridLayout::ComputeLayout(
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        int availableWidth,
        int availableHeight,
        const std::vector<Visual*>& children,
        int horizontalSpacing,
        int verticalSpacing)
    {
        auto effectiveRowDefs = GridLayout::ResolveDefinitions(rowDefs);
        auto effectiveColDefs = GridLayout::ResolveDefinitions(colDefs);

        const int gapX = std::max(0, horizontalSpacing);
        const int gapY = std::max(0, verticalSpacing);
        int numRows = std::max(1, static_cast<int>(effectiveRowDefs.size()));
        int numCols = std::max(1, static_cast<int>(effectiveColDefs.size()));

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid] ComputeLayout: Available %dx%d, Grid %dx%d, %zu children",
            availableWidth, availableHeight, numCols, numRows, children.size());

        GridLayoutResult result;
        result.rowHeights.resize(numRows, 0);
        result.colWidths.resize(numCols, 0);

        const int totalGapWidth = std::max(0, numCols - 1) * gapX;
        const int totalGapHeight = std::max(0, numRows - 1) * gapY;

        int totalFixedHeight = 0;
        int totalFixedWidth = 0;
        int starRowCount = 0;
        int starColCount = 0;

        for (int r = 0; r < numRows; ++r) {
            const auto& def = effectiveRowDefs[r];
            if (def.mode == SizeMode::Fixed) {
                int size = std::max(0, def.value);
                result.rowHeights[r] = size;
                totalFixedHeight += size;
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                    "[Grid] Row %d: Fixed %d", r, size);
            }
            else if (def.mode == SizeMode::Star) {
                starRowCount += def.value > 0 ? def.value : 1;
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                    "[Grid] Row %d: Star(%.0f)", r, (float)def.value);
            }
            else {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                    "[Grid] Row %d: Auto", r);
            }
        }

        for (int c = 0; c < numCols; ++c) {
            const auto& def = effectiveColDefs[c];
            if (def.mode == SizeMode::Fixed) {
                int size = std::max(0, def.value);
                result.colWidths[c] = size;
                totalFixedWidth += size;
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                    "[Grid] Col %d: Fixed %d", c, size);
            }
            else if (def.mode == SizeMode::Star) {
                starColCount += def.value > 0 ? def.value : 1;
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                    "[Grid] Col %d: Star(%.0f)", c, (float)def.value);
            }
            else {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                    "[Grid] Col %d: Auto", c);
            }
        }

        int remainingHeight = std::max(0, availableHeight - totalFixedHeight - totalGapHeight);
        int remainingWidth = std::max(0, availableWidth - totalFixedWidth - totalGapWidth);

        int starHeightPerUnit = starRowCount > 0 ? remainingHeight / starRowCount : 0;
        int starWidthPerUnit = starColCount > 0 ? remainingWidth / starColCount : 0;

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid] Star calculation: rows=%d(perUnit=%d), cols=%d(perUnit=%d)",
            starRowCount, starHeightPerUnit, starColCount, starWidthPerUnit);

        for (int r = 0; r < numRows; ++r) {
            const auto& def = effectiveRowDefs[r];
            if (def.mode == SizeMode::Star) {
                int starCount = def.value > 0 ? def.value : 1;
                result.rowHeights[r] = starHeightPerUnit * starCount;
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                    "[Grid] Row %d (Star): %d", r, result.rowHeights[r]);
            }
        }

        for (int c = 0; c < numCols; ++c) {
            const auto& def = effectiveColDefs[c];
            if (def.mode == SizeMode::Star) {
                int starCount = def.value > 0 ? def.value : 1;
                result.colWidths[c] = starWidthPerUnit * starCount;
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                    "[Grid] Col %d (Star): %d", c, result.colWidths[c]);
            }
        }

        int usedHeight = totalFixedHeight;
        for (int h : result.rowHeights) usedHeight += h;
        int heightRemainder = availableHeight - usedHeight;
        for (int r = 0; r < numRows && heightRemainder > 0; ++r) {
            if (effectiveRowDefs[r].mode == SizeMode::Star) {
                ++result.rowHeights[r];
                --heightRemainder;
            }
        }

        int usedWidth = totalFixedWidth;
        for (int w : result.colWidths) usedWidth += w;
        int widthRemainder = availableWidth - usedWidth;
        for (int c = 0; c < numCols && widthRemainder > 0; ++c) {
            if (effectiveColDefs[c].mode == SizeMode::Star) {
                ++result.colWidths[c];
                --widthRemainder;
            }
        }

        int totalH = 0, totalW = 0;
        for (int r = 0; r < numRows; ++r) totalH += result.rowHeights[r];
        for (int c = 0; c < numCols; ++c) totalW += result.colWidths[c];
        totalW += std::max(0, numCols - 1) * gapX;
        totalH += std::max(0, numRows - 1) * gapY;

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid] Layout (Auto=0): %dx%d", totalW, totalH);

        return result;
    }

    GridLayoutMetrics GridLayout::ComputeMetrics(
        const Grid* grid,
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        int availableWidth,
        int availableHeight,
        const std::vector<Visual*>& children,
        int horizontalSpacing,
        int verticalSpacing)
    {
        const int gapX = std::max(0, horizontalSpacing);
        const int gapY = std::max(0, verticalSpacing);

        GridLayoutMetrics metrics;
        metrics.autoRowHeights = ResolveAutoSizes(grid, rowDefs, children, true);
        metrics.autoColWidths = ResolveAutoSizes(grid, colDefs, children, false);

        auto resolvedRowDefs = NormalizeDefinitions(rowDefs, metrics.autoRowHeights);
        auto resolvedColDefs = NormalizeDefinitions(colDefs, metrics.autoColWidths);

        const auto layout = ComputeLayout(resolvedRowDefs, resolvedColDefs, availableWidth, availableHeight, children, gapX, gapY);
        metrics.rowHeights = layout.rowHeights;
        metrics.colWidths = layout.colWidths;
        metrics.childLayouts.resize(children.size());

        for (size_t i = 0; i < children.size(); ++i) {
            auto child = children[i];
            if (!child) {
                continue;
            }

            int r = grid ? grid->GetRow(child) : 0;
            int c = grid ? grid->GetColumn(child) : 0;
            int rspan = Grid::GetRowSpan(child);
            int cspan = Grid::GetColumnSpan(child);

            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;

            int boundedRow = std::max(0, r);
            int boundedCol = std::max(0, c);
            int safeRowSpan = std::max(1, rspan);
            int safeColSpan = std::max(1, cspan);
            int colCount = static_cast<int>(metrics.colWidths.size());
            int rowCount = static_cast<int>(metrics.rowHeights.size());
            int endCol = std::min(colCount, boundedCol + safeColSpan);
            int endRow = std::min(rowCount, boundedRow + safeRowSpan);

            for (int j = 0; j < boundedCol && j < colCount; ++j) {
                x += metrics.colWidths[j] + gapX;
            }
            for (int j = boundedCol; j < endCol; ++j) {
                w += metrics.colWidths[j];
            }
            if (safeColSpan > 1) {
                w += gapX * (safeColSpan - 1);
            }
            for (int j = 0; j < boundedRow && j < rowCount; ++j) {
                y += metrics.rowHeights[j] + gapY;
            }
            for (int j = boundedRow; j < endRow; ++j) {
                h += metrics.rowHeights[j];
            }
            if (safeRowSpan > 1) {
                h += gapY * (safeRowSpan - 1);
            }

            metrics.childLayouts[i] = { x, y, w, h };
        }

        return metrics;
    }

}