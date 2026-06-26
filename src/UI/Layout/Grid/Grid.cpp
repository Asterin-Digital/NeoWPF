#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/GridLayout.h>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <algorithm>

// Windows macro conflicts Start
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
// Windows macro conflicts End

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
    auto layout = GridLayout::ComputeLayout(m_rowDefs, m_colDefs, availableWidth, availableHeight, GetVisualChildren());

    int totalHeight = 0;
    int totalWidth = 0;
    for (int h : layout.rowHeights) totalHeight += h;
    for (int w : layout.colWidths) totalWidth += w;

    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            int c = GetColumn(child);
            int cspan = GetColumnSpan(child);
            int measWidth = 0;
            for (int i = c; i < c + cspan && i < static_cast<int>(layout.colWidths.size()); ++i) {
                measWidth += layout.colWidths[i];
            }

            int r = GetRow(child);
            int rspan = GetRowSpan(child);
            int measHeight = 0;
            for (int i = r; i < r + rspan && i < static_cast<int>(layout.rowHeights.size()); ++i) {
                measHeight += layout.rowHeights[i];
            }

            fe->Measure(measWidth, measHeight);
        }
    }

    SetDesiredSize(totalWidth, totalHeight);
}

void Grid::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    auto layout = GridLayout::ComputeLayout(m_rowDefs, m_colDefs, finalWidth, finalHeight, GetVisualChildren());

    for (auto child : GetVisualChildren()) {
        if (!child) continue;

        int r = GetRow(child);
        int c = GetColumn(child);
        int rspan = GetRowSpan(child);
        int cspan = GetColumnSpan(child);

        int x = 0, y = 0, w = 0, h = 0;

        for (int i = 0; i < c && i < static_cast<int>(layout.colWidths.size()); ++i) {
            x += layout.colWidths[i];
        }
        for (int i = c; i < c + cspan && i < static_cast<int>(layout.colWidths.size()); ++i) {
            w += layout.colWidths[i];
        }

        for (int i = 0; i < r && i < static_cast<int>(layout.rowHeights.size()); ++i) {
            y += layout.rowHeights[i];
        }
        for (int i = r; i < r + rspan && i < static_cast<int>(layout.rowHeights.size()); ++i) {
            h += layout.rowHeights[i];
        }

        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Arrange(w, h);
            fe->SetLayoutOffset(x, y);
        }
    }
}

}
