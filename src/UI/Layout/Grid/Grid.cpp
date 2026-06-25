#include <NeoWPF/UI/Layout/Grid/Grid.h>
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

void Grid::Measure(int availableWidth, int availableHeight)
{
    int rows = std::max(1, static_cast<int>(m_rowDefs.size()));
    int cols = std::max(1, static_cast<int>(m_colDefs.size()));

    int cellW = availableWidth / cols;
    int cellH = availableHeight / rows;

    for (auto child : GetVisualChildren()) {
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Measure(cellW, cellH);
        }
    }

    SetDesiredSize(availableWidth, availableHeight);
}

void Grid::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    int rows = std::max(1, static_cast<int>(m_rowDefs.size()));
    int cols = std::max(1, static_cast<int>(m_colDefs.size()));

    int cellW = finalWidth / cols;
    int cellH = finalHeight / rows;

    for (auto child : GetVisualChildren()) {
        if (!child) continue;
        int r = GetRow(child);
        int c = GetColumn(child);
        int x = c * cellW;
        int y = r * cellH;
        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            fe->Arrange(cellW, cellH);
            fe->SetLayoutOffset(x, y);
        }
    }
}

}
