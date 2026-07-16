#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/GridLayout.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Alignment/Alignment.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace {
void QueueGridDeveloperMessage(const std::wstring& code, const std::wstring& summary, const std::wstring& detail)
{
    NeoWPF::Diagnostics::QueueDeveloperWarning(
        code + L": " + summary,
        detail,
        NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
}
}

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

void Grid::AddChild(const std::shared_ptr<Visual>& child, int row, int column, int rowSpan, int columnSpan)
{
    if (!child) return;

    if (row < 0 || column < 0) {
        QueueGridDeveloperMessage(
            NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridIndexCode,
            L"invalid Grid row or column index",
            L"Grid children must use non-negative row and column indices.");
    }

    if (rowSpan <= 0 || columnSpan <= 0) {
        QueueGridDeveloperMessage(
            NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridSpanCode,
            L"invalid Grid span",
            L"Grid spans must be greater than zero.");
    }

    SetRow(child.get(), row);
    SetColumn(child.get(), column);
    SetRowSpan(child.get(), rowSpan);
    SetColumnSpan(child.get(), columnSpan);
    AddChild(child);
}

void Grid::Measure(int availableWidth, int availableHeight)
{
    if (GridLayout::s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid::Measure] Available: %dx%d", availableWidth, availableHeight);

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

        if (r < 0 || c < 0) {
            QueueGridDeveloperMessage(
                NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridIndexCode,
                L"invalid Grid row or column index",
                L"Grid child placement uses a negative row or column index.");
            continue;
        }

        if (rspan <= 0 || cspan <= 0) {
            QueueGridDeveloperMessage(
                NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridSpanCode,
                L"invalid Grid span",
                L"Grid child spans must be greater than zero.");
            continue;
        }

        if (r >= numRows || c >= numCols || r + rspan > numRows || c + cspan > numCols) {
            QueueGridDeveloperMessage(
                NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridIndexCode,
                L"Grid child is outside the defined rows or columns",
                L"The requested row, column, or span exceeds the current Grid definitions.");
            continue;
        }

        bool measureWidth = (cspan == 1 && c < numCols && m_colDefs[c].mode == SizeMode::Auto);
        bool measureHeight = (rspan == 1 && r < numRows && m_rowDefs[r].mode == SizeMode::Auto);

        if (measureWidth || measureHeight)
            fe->Measure(measureWidth ? INF : 0, measureHeight ? INF : 0);
    }

    auto children = GetVisualChildren();
    auto metrics = GridLayout::ComputeMetrics(
        m_rowDefs,
        m_colDefs,
        availableWidth,
        availableHeight,
        children);

    m_autoRowHeights = metrics.autoRowHeights;
    m_autoColWidths = metrics.autoColWidths;

    for (size_t i = 0; i < children.size(); ++i)
    {
        auto child = children[i];
        if (!child) continue;
        auto fe = dynamic_cast<FrameworkElement*>(child);
        if (!fe) continue;

        int r = GetRow(child);
        int c = GetColumn(child);
        int rspan = GetRowSpan(child);
        int cspan = GetColumnSpan(child);

        const auto& layout = metrics.childLayouts[i];
        fe->Measure(layout.width, layout.height);
        if (GridLayout::s_gridLogging) {
            NeoWPF::Diagnostics::LogInfo("[Grid::Measure] Child[%zu] at (%d,%d) span(%d,%d): cell %dx%d, desired %dx%d, horizontal=%d vertical=%d",
                i, r, c, rspan, cspan, layout.width, layout.height,
                fe->GetDesiredWidth(), fe->GetDesiredHeight(),
                static_cast<int>(fe->GetHorizontalAlignment()),
                static_cast<int>(fe->GetVerticalAlignment()));
        }
    }

    int totalW = 0, totalH = 0;
    for (int w : metrics.colWidths) totalW += w;
    for (int h : metrics.rowHeights) totalH += h;
    SetDesiredSize(totalW, totalH);
    if (GridLayout::s_gridLogging) NeoWPF::Diagnostics::LogInfo("[Grid::Measure] Final desired size: %dx%d", totalW, totalH);
}

void Grid::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    if (GridLayout::s_gridLogging)
        NeoWPF::Diagnostics::LogInfo("[Grid::Arrange] Final size: %dx%d", finalWidth, finalHeight);

    int numRows = std::max(1, static_cast<int>(m_rowDefs.size()));
    int numCols = std::max(1, static_cast<int>(m_colDefs.size()));


    auto children = GetVisualChildren();
    auto metrics = GridLayout::ComputeMetrics(
        m_rowDefs,
        m_colDefs,
        finalWidth,
        finalHeight,
        children);

    for (size_t i = 0; i < children.size(); ++i) {
        auto child = children[i];
        if (!child) continue;

        int r = GetRow(child);
        int c = GetColumn(child);
        int rspan = GetRowSpan(child);
        int cspan = GetColumnSpan(child);

        const auto& layout = metrics.childLayouts[i];
        int x = layout.x;
        int y = layout.y;
        int w = layout.width;
        int h = layout.height;

        auto fe = dynamic_cast<FrameworkElement*>(child);
        if (!fe) continue;


        int desiredW = fe->GetDesiredWidth();
        int desiredH = fe->GetDesiredHeight();


        int alignedX = 0, alignedY = 0;
        int alignedW = desiredW;
        int alignedH = desiredH;


        switch (fe->GetHorizontalAlignment()) {
            case HorizontalAlignment::Left:
                alignedX = 0;
                alignedW = desiredW;
                break;
            case HorizontalAlignment::Center:
                alignedX = (w - desiredW) / 2;
                alignedW = desiredW;
                break;
            case HorizontalAlignment::Right:
                alignedX = w - desiredW;
                alignedW = desiredW;
                break;
            case HorizontalAlignment::Stretch:
                alignedX = 0;
                alignedW = w;
                break;
        }


        switch (fe->GetVerticalAlignment()) {
            case VerticalAlignment::Top:
                alignedY = 0;
                alignedH = desiredH;
                break;
            case VerticalAlignment::Center:
                alignedY = (h - desiredH) / 2;
                alignedH = desiredH;
                break;
            case VerticalAlignment::Bottom:
                alignedY = h - desiredH;
                alignedH = desiredH;
                break;
            case VerticalAlignment::Stretch:
                alignedY = 0;
                alignedH = h;
                break;
        }


        fe->Arrange(alignedW, alignedH);

        const int targetX = x + alignedX;
        const int targetY = y + alignedY;
        const int currentX = fe->GetLayoutOffsetX();
        const int currentY = fe->GetLayoutOffsetY();
        if (currentX != targetX || currentY != targetY) {
            fe->SetLayoutOffset(targetX, targetY);
        }

        if (GridLayout::s_gridLogging) {
            NeoWPF::Diagnostics::LogInfo(
                "[Grid::Arrange] Child[%zu] at (%d,%d): pos(%d,%d -> %d,%d) size(%dx%d) [cell %dx%d] desired=%dx%d",
                i, r, c, currentX, currentY, targetX, targetY, alignedW, alignedH, w, h,
                desiredW, desiredH);
        }
    }
}

}