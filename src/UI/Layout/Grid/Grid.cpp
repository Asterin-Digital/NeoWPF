#include <NeoWPF/UI/Layout/Grid/Grid.h>
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

std::unordered_map<Visual*, int> Grid::s_rowMap;
std::unordered_map<Visual*, int> Grid::s_colMap;
std::unordered_map<Visual*, int> Grid::s_rowSpanMap;
std::unordered_map<Visual*, int> Grid::s_colSpanMap;

void Grid::SetRow(Visual* v, int r)
{
    if (!v) return;
    s_rowMap[v] = r;
}

void Grid::SetColumn(Visual* v, int c)
{
    if (!v) return;
    s_colMap[v] = c;
}

int Grid::GetRow(Visual* v)
{
    if (!v) return 0;
    auto it = s_rowMap.find(v);
    return it == s_rowMap.end() ? 0 : it->second;
}

int Grid::GetColumn(Visual* v)
{
    if (!v) return 0;
    auto it = s_colMap.find(v);
    return it == s_colMap.end() ? 0 : it->second;
}

void Grid::SetRowSpan(Visual* v, int span)
{
    if (!v) return;
    s_rowSpanMap[v] = span;
}

void Grid::SetColumnSpan(Visual* v, int span)
{
    if (!v) return;
    s_colSpanMap[v] = span;
}

int Grid::GetRowSpan(Visual* v)
{
    if (!v) return 1;
    auto it = s_rowSpanMap.find(v);
    return it == s_rowSpanMap.end() ? 1 : it->second;
}

int Grid::GetColumnSpan(Visual* v)
{
    if (!v) return 1;
    auto it = s_colSpanMap.find(v);
    return it == s_colSpanMap.end() ? 1 : it->second;
}

void Grid::Measure(int availableWidth, int availableHeight)
{
    NeoWPF::Diagnostics::LogInfo("[Grid::Measure] Available: %dx%d", availableWidth, availableHeight);

    const int INF = 1000000;
    int numRows = std::max(1, static_cast<int>(m_rowDefs.size()));
    int numCols = std::max(1, static_cast<int>(m_colDefs.size()));

    for (auto child : GetVisualChildren())
    {
        if (!child) continue;
        auto fe = dynamic_cast<FrameworkElement*>(child);
        if (!fe) continue;
        int r = GetRow(child);
        int c = GetColumn(child);
        int rspan = GetRowSpan(child);
        int cspan = GetColumnSpan(child);

        bool measureWidth = (cspan == 1 && c < numCols && m_colDefs[c].mode == SizeMode::Auto);
        bool measureHeight = (rspan == 1 && r < numRows && m_rowDefs[r].mode == SizeMode::Auto);

        if (measureWidth || measureHeight)
            fe->Measure(measureWidth ? INF : 0, measureHeight ? INF : 0);
    }

    std::vector<int> autoRowHeights(numRows, 0);
    std::vector<int> autoColWidths(numCols, 0);

    for (int r = 0; r < numRows; ++r)
    {
        if (m_rowDefs[r].mode == SizeMode::Auto)
        {
            int maxH = 0;
            for (auto child : GetVisualChildren())
            {
                if (!child) continue;
                if (GetRow(child) == r && GetRowSpan(child) == 1)
                {
                    auto fe = dynamic_cast<FrameworkElement*>(child);
                    if (fe) maxH = std::max(maxH, fe->GetDesiredHeight());
                }
            }
            autoRowHeights[r] = maxH;
            NeoWPF::Diagnostics::LogInfo("[Grid::Measure] Row %d Auto desired = %d", r, maxH);
        }
    }

    for (int c = 0; c < numCols; ++c)
    {
        if (m_colDefs[c].mode == SizeMode::Auto)
        {
            int maxW = 0;
            for (auto child : GetVisualChildren())
            {
                if (!child) continue;
                if (GetColumn(child) == c && GetColumnSpan(child) == 1)
                {
                    auto fe = dynamic_cast<FrameworkElement*>(child);
                    if (fe) maxW = std::max(maxW, fe->GetDesiredWidth());
                }
            }
            autoColWidths[c] = maxW;
            NeoWPF::Diagnostics::LogInfo("[Grid::Measure] Col %d Auto desired = %d", c, maxW);
        }
    }

    std::vector<Definition> measureRowDefs = m_rowDefs;
    std::vector<Definition> measureColDefs = m_colDefs;
    for (int r = 0; r < numRows; ++r)
    {
        if (measureRowDefs[r].mode == SizeMode::Auto)
        {
            measureRowDefs[r].mode = SizeMode::Fixed;
            measureRowDefs[r].value = autoRowHeights[r];
        }
    }
    for (int c = 0; c < numCols; ++c)
    {
        if (measureColDefs[c].mode == SizeMode::Auto)
        {
            measureColDefs[c].mode = SizeMode::Fixed;
            measureColDefs[c].value = autoColWidths[c];
        }
    }

    auto layout = GridLayout::ComputeLayout(measureRowDefs, measureColDefs,
                                            availableWidth, availableHeight,
                                            GetVisualChildren());

    for (size_t i = 0; i < GetVisualChildren().size(); ++i)
    {
        auto child = GetVisualChildren()[i];
        if (!child) continue;
        auto fe = dynamic_cast<FrameworkElement*>(child);
        if (!fe) continue;

        int r = GetRow(child);
        int c = GetColumn(child);
        int rspan = GetRowSpan(child);
        int cspan = GetColumnSpan(child);

        int cellW = 0, cellH = 0;
        for (int j = c; j < c + cspan && j < static_cast<int>(layout.colWidths.size()); ++j)
            cellW += layout.colWidths[j];
        for (int j = r; j < r + rspan && j < static_cast<int>(layout.rowHeights.size()); ++j)
            cellH += layout.rowHeights[j];

        fe->Measure(cellW, cellH);
        NeoWPF::Diagnostics::LogInfo("[Grid::Measure] Child[%zu] at (%d,%d) span(%d,%d): measured %dx%d, desired %dx%d",
            i, r, c, rspan, cspan, cellW, cellH,
            fe->GetDesiredWidth(), fe->GetDesiredHeight());
    }

    int totalW = 0, totalH = 0;
    for (int w : layout.colWidths) totalW += w;
    for (int h : layout.rowHeights) totalH += h;
    SetDesiredSize(totalW, totalH);
    NeoWPF::Diagnostics::LogInfo("[Grid::Measure] Final desired size: %dx%d", totalW, totalH);
}

void Grid::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    NeoWPF::Diagnostics::LogInfo("[Grid::Arrange] Final size: %dx%d", finalWidth, finalHeight);

    int numRows = std::max(1, static_cast<int>(m_rowDefs.size()));
    int numCols = std::max(1, static_cast<int>(m_colDefs.size()));

    std::vector<int> autoRowHeights(numRows, 0);
    std::vector<int> autoColWidths(numCols, 0);

    for (int r = 0; r < numRows; ++r)
    {
        if (m_rowDefs[r].mode == SizeMode::Auto)
        {
            int maxH = 0;
            for (auto child : GetVisualChildren())
            {
                if (!child) continue;
                if (GetRow(child) == r && GetRowSpan(child) == 1)
                {
                    auto fe = dynamic_cast<FrameworkElement*>(child);
                    if (fe) maxH = std::max(maxH, fe->GetDesiredHeight());
                }
            }
            autoRowHeights[r] = maxH;
            NeoWPF::Diagnostics::LogInfo("[Grid::Arrange] Row %d Auto desired = %d", r, maxH);
        }
    }

    for (int c = 0; c < numCols; ++c)
    {
        if (m_colDefs[c].mode == SizeMode::Auto)
        {
            int maxW = 0;
            for (auto child : GetVisualChildren())
            {
                if (!child) continue;
                if (GetColumn(child) == c && GetColumnSpan(child) == 1)
                {
                    auto fe = dynamic_cast<FrameworkElement*>(child);
                    if (fe) maxW = std::max(maxW, fe->GetDesiredWidth());
                }
            }
            autoColWidths[c] = maxW;
            NeoWPF::Diagnostics::LogInfo("[Grid::Arrange] Col %d Auto desired = %d", c, maxW);
        }
    }

    std::vector<Definition> arrangeRowDefs = m_rowDefs;
    std::vector<Definition> arrangeColDefs = m_colDefs;
    for (int r = 0; r < numRows; ++r)
    {
        if (arrangeRowDefs[r].mode == SizeMode::Auto)
        {
            arrangeRowDefs[r].mode = SizeMode::Fixed;
            arrangeRowDefs[r].value = autoRowHeights[r];
        }
    }
    for (int c = 0; c < numCols; ++c)
    {
        if (arrangeColDefs[c].mode == SizeMode::Auto)
        {
            arrangeColDefs[c].mode = SizeMode::Fixed;
            arrangeColDefs[c].value = autoColWidths[c];
        }
    }

    auto layout = GridLayout::ComputeLayout(arrangeRowDefs, arrangeColDefs,
                                            finalWidth, finalHeight,
                                            GetVisualChildren());

    for (size_t i = 0; i < GetVisualChildren().size(); ++i)
    {
        auto child = GetVisualChildren()[i];
        if (!child) continue;

        int r = GetRow(child);
        int c = GetColumn(child);
        int rspan = GetRowSpan(child);
        int cspan = GetColumnSpan(child);

        int x = 0, y = 0, w = 0, h = 0;
        for (int j = 0; j < c && j < static_cast<int>(layout.colWidths.size()); ++j)
            x += layout.colWidths[j];
        for (int j = c; j < c + cspan && j < static_cast<int>(layout.colWidths.size()); ++j)
            w += layout.colWidths[j];
        for (int j = 0; j < r && j < static_cast<int>(layout.rowHeights.size()); ++j)
            y += layout.rowHeights[j];
        for (int j = r; j < r + rspan && j < static_cast<int>(layout.rowHeights.size()); ++j)
            h += layout.rowHeights[j];

        if (auto fe = dynamic_cast<FrameworkElement*>(child))
        {
            fe->Arrange(w, h);
            fe->SetLayoutOffset(x, y);
            NeoWPF::Diagnostics::LogInfo("[Grid::Arrange] Child[%zu] at (%d,%d): pos(%d,%d) size(%dx%d)",
                i, r, c, x, y, w, h);
        }
    }
}

}