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
            std::vector<Grid::Definition> normalized = defs;
            if (normalized.empty()) {
                normalized.push_back({ SizeMode::Auto, 0 });
            }

            for (size_t i = 0; i < normalized.size(); ++i) {
                if (normalized[i].mode == SizeMode::Auto) {
                    normalized[i].mode = SizeMode::Fixed;
                    normalized[i].value = i < autoSizes.size() ? autoSizes[i] : 0;
                }
            }

            return normalized;
        }

        std::vector<int> ResolveAutoSizes(
            const std::vector<Grid::Definition>& defs,
            const std::vector<Visual*>& children,
            bool resolveRowHeights)
        {
            const int count = std::max(1, static_cast<int>(defs.size()));
            std::vector<int> autoSizes(count, 0);

            for (int index = 0; index < count; ++index) {
                const bool isAuto = defs.empty() || defs[index].mode == SizeMode::Auto || defs[index].mode == SizeMode::Star;
                if (!isAuto) {
                    continue;
                }

                int maxSize = 0;
                for (auto child : children) {
                    if (!child) {
                        continue;
                    }

                    if (resolveRowHeights) {
                        if (Grid::GetRow(child) == index && Grid::GetRowSpan(child) == 1) {
                            if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
                                const auto margin = fe->GetMargin();
                                maxSize = std::max(maxSize, fe->GetDesiredHeight() + margin.Vertical());
                            }
                        }
                    }
                    else {
                        if (Grid::GetColumn(child) == index && Grid::GetColumnSpan(child) == 1) {
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

    bool GridLayout::s_gridLogging = true; // Please do not turn this on unless ur debugging something

    GridLayoutResult GridLayout::ComputeLayout(
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        int availableWidth,
        int availableHeight,
        const std::vector<Visual*>& children)
    {
        std::vector<Grid::Definition> effectiveRowDefs = rowDefs;
        std::vector<Grid::Definition> effectiveColDefs = colDefs;
        if (effectiveRowDefs.empty()) {
            effectiveRowDefs.push_back({ SizeMode::Auto, 0 });
        }
        if (effectiveColDefs.empty()) {
            effectiveColDefs.push_back({ SizeMode::Auto, 0 });
        }

        int numRows = std::max(1, static_cast<int>(effectiveRowDefs.size()));
        int numCols = std::max(1, static_cast<int>(effectiveColDefs.size()));

        if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] ComputeLayout: Available %dx%d, Grid %dx%d, %zu children",
            availableWidth, availableHeight, numCols, numRows, children.size());

        GridLayoutResult result;
        result.rowHeights.resize(numRows, 0);
        result.colWidths.resize(numCols, 0);

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
                if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Row %d: Fixed %d", r, size);
            }
            else if (def.mode == SizeMode::Star) {
                starRowCount += def.value > 0 ? def.value : 1;
                if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Row %d: Star(%.0f)", r, (float)def.value);
            }
            else {
                if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Row %d: Auto", r);
            }
        }

        for (int c = 0; c < numCols; ++c) {
            const auto& def = effectiveColDefs[c];
            if (def.mode == SizeMode::Fixed) {
                int size = std::max(0, def.value);
                result.colWidths[c] = size;
                totalFixedWidth += size;
                if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Col %d: Fixed %d", c, size);
            }
            else if (def.mode == SizeMode::Star) {
                starColCount += def.value > 0 ? def.value : 1;
                if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Col %d: Star(%.0f)", c, (float)def.value);
            }
            else {
                if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Col %d: Auto", c);
            }
        }

        int remainingHeight = std::max(0, availableHeight - totalFixedHeight);
        int remainingWidth = std::max(0, availableWidth - totalFixedWidth);

        int starHeightPerUnit = starRowCount > 0 ? remainingHeight / starRowCount : 0;
        int starWidthPerUnit = starColCount > 0 ? remainingWidth / starColCount : 0;

        if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Star calculation: rows=%d(perUnit=%d), cols=%d(perUnit=%d)",
            starRowCount, starHeightPerUnit, starColCount, starWidthPerUnit);

        for (int r = 0; r < numRows; ++r) {
            const auto& def = effectiveRowDefs[r];
            if (def.mode == SizeMode::Star) {
                int starCount = def.value > 0 ? def.value : 1;
                result.rowHeights[r] = starHeightPerUnit * starCount;
                if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Row %d (Star): %d", r, result.rowHeights[r]);
            }
        }

        for (int c = 0; c < numCols; ++c) {
            const auto& def = effectiveColDefs[c];
            if (def.mode == SizeMode::Star) {
                int starCount = def.value > 0 ? def.value : 1;
                result.colWidths[c] = starWidthPerUnit * starCount;
                if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Col %d (Star): %d", c, result.colWidths[c]);
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

        if (s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid] Layout (Auto=0): %dx%d", totalW, totalH);

        return result;
    }

    GridLayoutMetrics GridLayout::ComputeMetrics(
        const std::vector<Grid::Definition>& rowDefs,
        const std::vector<Grid::Definition>& colDefs,
        int availableWidth,
        int availableHeight,
        const std::vector<Visual*>& children)
    {
        GridLayoutMetrics metrics;
        metrics.autoRowHeights = ResolveAutoSizes(rowDefs, children, true);
        metrics.autoColWidths = ResolveAutoSizes(colDefs, children, false);

        auto resolvedRowDefs = NormalizeDefinitions(rowDefs, metrics.autoRowHeights);
        auto resolvedColDefs = NormalizeDefinitions(colDefs, metrics.autoColWidths);

        const auto layout = ComputeLayout(resolvedRowDefs, resolvedColDefs, availableWidth, availableHeight, children);
        metrics.rowHeights = layout.rowHeights;
        metrics.colWidths = layout.colWidths;
        metrics.childLayouts.resize(children.size());

        for (size_t i = 0; i < children.size(); ++i) {
            auto child = children[i];
            if (!child) {
                continue;
            }

            int r = Grid::GetRow(child);
            int c = Grid::GetColumn(child);
            int rspan = Grid::GetRowSpan(child);
            int cspan = Grid::GetColumnSpan(child);

            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;

            for (int j = 0; j < c && j < static_cast<int>(metrics.colWidths.size()); ++j) {
                x += metrics.colWidths[j];
            }
            for (int j = c; j < c + cspan && j < static_cast<int>(metrics.colWidths.size()); ++j) {
                w += metrics.colWidths[j];
            }
            for (int j = 0; j < r && j < static_cast<int>(metrics.rowHeights.size()); ++j) {
                y += metrics.rowHeights[j];
            }
            for (int j = r; j < r + rspan && j < static_cast<int>(metrics.rowHeights.size()); ++j) {
                h += metrics.rowHeights[j];
            }

            metrics.childLayouts[i] = { x, y, w, h };
        }

        return metrics;
    }

}