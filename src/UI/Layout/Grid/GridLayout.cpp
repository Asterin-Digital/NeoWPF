#include <NeoWPF/UI/Layout/Grid/GridLayout.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <cmath>

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
            const std::vector<double>& autoSizes)
        {
            std::vector<Grid::Definition> normalized = GridLayout::ResolveDefinitions(defs);

            for (std::size_t i = 0; i < normalized.size(); ++i) {
                if (normalized[i].mode == SizeMode::Auto) {
                    normalized[i].mode = SizeMode::Fixed;
                    const double autoSize = i < autoSizes.size() ? (autoSizes[i] > 0.0 ? autoSizes[i] : 0.0) : 0.0;
                    normalized[i].value = static_cast<int>(std::lround(autoSize));
                }
            }

            return normalized;
        }

        std::vector<double> ResolveAutoSizes(
            const Grid* grid,
            const std::vector<Grid::Definition>& defs,
            const std::vector<Visual*>& children,
            bool resolveRowHeights)
        {
            const auto effectiveDefs = GridLayout::ResolveDefinitions(defs);
            const std::size_t count = effectiveDefs.empty() ? 1u : effectiveDefs.size();
            std::vector<double> autoSizes(count, 0.0);

            for (std::size_t index = 0; index < count; ++index) {
                const bool isAuto = effectiveDefs[index].mode == SizeMode::Auto || effectiveDefs[index].mode == SizeMode::Star;
                if (!isAuto) {
                    continue;
                }

                double maxSize = 0.0;
                for (auto child : children) {
                    if (!child || !grid) {
                        continue;
                    }

                    if (resolveRowHeights) {
                        if (grid->GetRow(child) == index && Grid::GetRowSpan(child) == 1) {
                            if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
                                const auto margin = fe->GetMargin();
                                const double candidateHeight = fe->GetDesiredHeight() + margin.Vertical();
                                maxSize = candidateHeight > maxSize ? candidateHeight : maxSize;
                            }
                        }
                    }
                    else {
                        if (grid->GetColumn(child) == index && Grid::GetColumnSpan(child) == 1) {
                            if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
                                const auto margin = fe->GetMargin();
                                const double candidateWidth = fe->GetDesiredWidth() + margin.Horizontal();
                                maxSize = candidateWidth > maxSize ? candidateWidth : maxSize;
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
        double availableWidth,
        double availableHeight,
        const std::vector<Visual*>& children,
        double horizontalSpacing,
        double verticalSpacing)
    {
        auto effectiveRowDefs = GridLayout::ResolveDefinitions(rowDefs);
        auto effectiveColDefs = GridLayout::ResolveDefinitions(colDefs);

        const double gapX = horizontalSpacing > 0.0 ? horizontalSpacing : 0.0;
        const double gapY = verticalSpacing > 0.0 ? verticalSpacing : 0.0;
        const std::size_t numRows = effectiveRowDefs.empty() ? 1u : effectiveRowDefs.size();
        const std::size_t numCols = effectiveColDefs.empty() ? 1u : effectiveColDefs.size();

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid] ComputeLayout: Available %gx%g, Grid %zux%zu, %zu children",
            availableWidth, availableHeight, numCols, numRows, children.size());

        GridLayoutResult result;
        result.rowHeights.resize(numRows, 0.0);
        result.colWidths.resize(numCols, 0.0);

        const double totalGapWidth = numCols > 1u ? (numCols - 1u) * gapX : 0.0;
        const double totalGapHeight = numRows > 1u ? (numRows - 1u) * gapY : 0.0;

        double totalFixedHeight = 0.0;
        double totalFixedWidth = 0.0;
        double starRowCount = 0.0;
        double starColCount = 0.0;

        for (std::size_t r = 0; r < numRows; ++r) {
            const auto& def = effectiveRowDefs[r];
            if (def.mode == SizeMode::Fixed) {
                const double size = def.value > 0 ? def.value : 0.0;
                result.rowHeights[r] = size;
                totalFixedHeight += size;
            }
            else if (def.mode == SizeMode::Star) {
                starRowCount += def.value > 0 ? def.value : 1.0;
            }
        }

        for (std::size_t c = 0; c < numCols; ++c) {
            const auto& def = effectiveColDefs[c];
            if (def.mode == SizeMode::Fixed) {
                const double size = def.value > 0 ? def.value : 0.0;
                result.colWidths[c] = size;
                totalFixedWidth += size;
            }
            else if (def.mode == SizeMode::Star) {
                starColCount += def.value > 0 ? def.value : 1.0;
            }
        }

        const double remainingHeightValue = availableHeight - totalFixedHeight - totalGapHeight;
        double remainingHeight = remainingHeightValue > 0.0 ? remainingHeightValue : 0.0;
        const double remainingWidthValue = availableWidth - totalFixedWidth - totalGapWidth;
        double remainingWidth = remainingWidthValue > 0.0 ? remainingWidthValue : 0.0;

        const double starHeightPerUnit = starRowCount > 0.0 ? std::floor(remainingHeight / starRowCount) : 0.0;
        const double starWidthPerUnit = starColCount > 0.0 ? std::floor(remainingWidth / starColCount) : 0.0;

        for (std::size_t r = 0; r < numRows; ++r) {
            const auto& def = effectiveRowDefs[r];
            if (def.mode == SizeMode::Star) {
                const double starCount = def.value > 0 ? def.value : 1.0;
                result.rowHeights[r] = starHeightPerUnit * starCount;
            }
        }

        for (std::size_t c = 0; c < numCols; ++c) {
            const auto& def = effectiveColDefs[c];
            if (def.mode == SizeMode::Star) {
                const double starCount = def.value > 0 ? def.value : 1.0;
                result.colWidths[c] = starWidthPerUnit * starCount;
            }
        }

        double usedHeight = totalFixedHeight;
        for (const double h : result.rowHeights) usedHeight += h;
        double heightRemainder = availableHeight - usedHeight;
        for (std::size_t r = 0; r < numRows && heightRemainder > 0.0; ++r) {
            if (effectiveRowDefs[r].mode == SizeMode::Star) {
                result.rowHeights[r] += 1.0;
                heightRemainder -= 1.0;
            }
        }

        double usedWidth = totalFixedWidth;
        for (const double w : result.colWidths) usedWidth += w;
        double widthRemainder = availableWidth - usedWidth;
        for (std::size_t c = 0; c < numCols && widthRemainder > 0.0; ++c) {
            if (effectiveColDefs[c].mode == SizeMode::Star) {
                result.colWidths[c] += 1.0;
                widthRemainder -= 1.0;
            }
        }

        double totalH = 0.0;
        double totalW = 0.0;
        for (std::size_t r = 0; r < numRows; ++r) totalH += result.rowHeights[r];
        for (std::size_t c = 0; c < numCols; ++c) totalW += result.colWidths[c];
        totalW += numCols > 1u ? (numCols - 1u) * gapX : 0.0;
        totalH += numRows > 1u ? (numRows - 1u) * gapY : 0.0;

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid] Layout (Auto=0): %gx%g", totalW, totalH);

        return result;
    }

    GridLayoutMetrics GridLayout::ComputeMetrics(
        const Grid* grid,
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        double availableWidth,
        double availableHeight,
        const std::vector<Visual*>& children,
        double horizontalSpacing,
        double verticalSpacing)
    {
        const double gapX = horizontalSpacing > 0.0 ? horizontalSpacing : 0.0;
        const double gapY = verticalSpacing > 0.0 ? verticalSpacing : 0.0;

        GridLayoutMetrics metrics;
        metrics.autoRowHeights = ResolveAutoSizes(grid, rowDefs, children, true);
        metrics.autoColWidths = ResolveAutoSizes(grid, colDefs, children, false);

        auto resolvedRowDefs = NormalizeDefinitions(rowDefs, metrics.autoRowHeights);
        auto resolvedColDefs = NormalizeDefinitions(colDefs, metrics.autoColWidths);

        const auto layout = ComputeLayout(resolvedRowDefs, resolvedColDefs, availableWidth, availableHeight, children, gapX, gapY);
        metrics.rowHeights = layout.rowHeights;
        metrics.colWidths = layout.colWidths;
        metrics.childLayouts.resize(children.size());

        const std::size_t colCount = metrics.colWidths.size();
        const std::size_t rowCount = metrics.rowHeights.size();

        for (std::size_t i = 0; i < children.size(); ++i) {
            auto child = children[i];
            if (!child) {
                continue;
            }

            const int rowPos = grid ? grid->GetRow(child) : 0;
            const int columnPos = grid ? grid->GetColumn(child) : 0;
            const int rowSpanValue = Grid::GetRowSpan(child);
            const int columnSpanValue = Grid::GetColumnSpan(child);

            double left = 0.0;
            double top = 0.0;
            double widthValue = 0.0;
            double heightValue = 0.0;

            const int boundedRow = rowPos > 0 ? rowPos : 0;
            const int boundedCol = columnPos > 0 ? columnPos : 0;
            const int safeRowSpan = rowSpanValue > 1 ? rowSpanValue : 1;
            const int safeColSpan = columnSpanValue > 1 ? columnSpanValue : 1;
            const int colCount = static_cast<int>(metrics.colWidths.size());
            const int rowCount = static_cast<int>(metrics.rowHeights.size());
            const int endCol = (boundedCol + safeColSpan) < colCount ? (boundedCol + safeColSpan) : colCount;
            const int endRow = (boundedRow + safeRowSpan) < rowCount ? (boundedRow + safeRowSpan) : rowCount;

            for (int j = 0; j < boundedCol && j < colCount; ++j) {
                left += metrics.colWidths[j] + gapX;
            }
            for (int j = boundedCol; j < endCol; ++j) {
                widthValue += metrics.colWidths[j];
            }
            if (safeColSpan > 1) {
                widthValue += gapX * (safeColSpan - 1);
            }
            for (int j = 0; j < boundedRow && j < rowCount; ++j) {
                top += metrics.rowHeights[j] + gapY;
            }
            for (int j = boundedRow; j < endRow; ++j) {
                heightValue += metrics.rowHeights[j];
            }
            if (safeRowSpan > 1) {
                heightValue += gapY * (safeRowSpan - 1);
            }

            metrics.childLayouts[i] = { left, top, widthValue, heightValue };
        }

        return metrics;
    }

}