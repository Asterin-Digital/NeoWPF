#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/GridLayout.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
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

    std::unordered_map<Visual*, int> Grid::s_rowSpanMap;
    std::unordered_map<Visual*, int> Grid::s_colSpanMap;

    void Grid::SetRow(Visual* v, int r) noexcept
    {
        if (!v) return;
        if (r <= 0) {
            m_rowMap.erase(v);
        } else {
            m_rowMap[v] = r;
        }
    }

    void Grid::SetColumn(Visual* v, int c) noexcept
    {
        if (!v) return;
        if (c <= 0) {
            m_colMap.erase(v);
        } else {
            m_colMap[v] = c;
        }
    }

    int Grid::GetRow(Visual* v) const noexcept
    {
        if (!v) return 0;
        auto it = m_rowMap.find(v);
        return it == m_rowMap.end() ? 0 : it->second;
    }

    int Grid::GetColumn(Visual* v) const noexcept
    {
        if (!v) return 0;
        auto it = m_colMap.find(v);
        return it == m_colMap.end() ? 0 : it->second;
    }

    void Grid::ClearAttachedProperties(Visual* v)
    {
        if (!v) return;
        m_rowMap.erase(v);
        m_colMap.erase(v);
        s_rowSpanMap.erase(v);
        s_colSpanMap.erase(v);
    }

    void Grid::OnRemoveVisualChild(Visual* child)
    {
        ClearAttachedProperties(child);
    }

    void Grid::SetRowSpan(Visual* v, int span)
    {
        if (!v) return;
        if (span <= 1) {
            s_rowSpanMap.erase(v);
        } else {
            s_rowSpanMap[v] = span;
        }
    }

    void Grid::SetColumnSpan(Visual* v, int span)
    {
        if (!v) return;
        if (span <= 1) {
            s_colSpanMap.erase(v);
        } else {
            s_colSpanMap[v] = span;
        }
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
        if (!child) {
            QueueGridDeveloperMessage(
                NeoWPF::Diagnostics::DeveloperWarnings::kNullGridChildCode,
                L"attempted to add a null child to a Grid",
                L"Grid::AddChild was called with a null child pointer; the call was ignored.");
            return;
        }

        if (child.get() == static_cast<Visual*>(this)) {
            QueueGridDeveloperMessage(
                NeoWPF::Diagnostics::DeveloperWarnings::kGridSelfParentingCode,
                L"attempted to add a Grid as a child of itself",
                L"A Grid cannot be added as its own child; this would cause infinite recursion during Measure/Arrange. The call was ignored.");
            return;
        }

        if (row < 0 || column < 0) {
            QueueGridDeveloperMessage(
                NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridIndexCode,
                L"invalid Grid row or column index",
                L"Grid children must use non-negative row and column indices.");
            return;
        }

        if (rowSpan <= 0 || columnSpan <= 0) {
            QueueGridDeveloperMessage(
                NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridSpanCode,
                L"invalid Grid span",
                L"Grid spans must be greater than zero.");
            return;
        }

        SetRow(child.get(), row);
        SetColumn(child.get(), column);
        SetRowSpan(child.get(), rowSpan);
        SetColumnSpan(child.get(), columnSpan);
        AddChild(child);
    }

    void Grid::Measure(int availableWidth, int availableHeight)
    {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid::Measure] Available: %dx%d", availableWidth, availableHeight);

        const int INF = 1000000;
        int numRows = std::max(1, static_cast<int>(m_rowDefs.size()));
        int numCols = std::max(1, static_cast<int>(m_colDefs.size()));

        for (size_t i = 0; i < m_rowDefs.size(); ++i) {
            const auto& def = m_rowDefs[i];
            if ((def.mode == SizeMode::Fixed || def.mode == SizeMode::Star) && def.value < 0) {
                QueueGridDeveloperMessage(
                    NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridDefinitionCode,
                    L"Grid RowDefinition has a negative size",
                    L"A Grid RowDefinition was given a negative Fixed or Star value; it will be treated as 0.");
            }
        }
        for (size_t i = 0; i < m_colDefs.size(); ++i) {
            const auto& def = m_colDefs[i];
            if ((def.mode == SizeMode::Fixed || def.mode == SizeMode::Star) && def.value < 0) {
                QueueGridDeveloperMessage(
                    NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridDefinitionCode,
                    L"Grid ColumnDefinition has a negative size",
                    L"A Grid ColumnDefinition was given a negative Fixed or Star value; it will be treated as 0.");
            }
        }

        for (auto child : GetVisualChildren())
        {
            if (!child) continue;
            auto fe = dynamic_cast<FrameworkElement*>(child);
            if (!fe) {
                QueueGridDeveloperMessage(
                    NeoWPF::Diagnostics::DeveloperWarnings::kGridChildTypeMismatchCode,
                    L"Grid child is not a FrameworkElement",
                    L"A child added to a Grid does not derive from FrameworkElement, so it cannot be measured or arranged and will not participate in layout.");
                continue;
            }
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

            bool rowOutOfRange = (r >= numRows) || (rspan > numRows - r);
            bool colOutOfRange = (c >= numCols) || (cspan > numCols - c);
            if (rowOutOfRange || colOutOfRange) {
                QueueGridDeveloperMessage(
                    NeoWPF::Diagnostics::DeveloperWarnings::kInvalidGridIndexCode,
                    L"Grid child is outside the defined rows or columns",
                    L"The requested row, column, or span exceeds the current Grid definitions.");
                continue;
            }

            const auto effectiveRowDefs = GridLayout::ResolveDefinitions(m_rowDefs);
            const auto effectiveColDefs = GridLayout::ResolveDefinitions(m_colDefs);
            const int effectiveNumRows = std::max(1, static_cast<int>(effectiveRowDefs.size()));
            const int effectiveNumCols = std::max(1, static_cast<int>(effectiveColDefs.size()));

            bool measureWidth = (cspan == 1 && c < effectiveNumCols && c < static_cast<int>(effectiveColDefs.size()) && effectiveColDefs[c].mode == SizeMode::Auto);
            bool measureHeight = (rspan == 1 && r < effectiveNumRows && r < static_cast<int>(effectiveRowDefs.size()) && effectiveRowDefs[r].mode == SizeMode::Auto);

            if (measureWidth || measureHeight)
                fe->Measure(measureWidth ? INF : 0, measureHeight ? INF : 0);
        }

        const auto effectiveRowDefs = GridLayout::ResolveDefinitions(m_rowDefs);
        const auto effectiveColDefs = GridLayout::ResolveDefinitions(m_colDefs);
        const int effectiveNumRows = std::max(1, static_cast<int>(effectiveRowDefs.size()));
        const int effectiveNumCols = std::max(1, static_cast<int>(effectiveColDefs.size()));
        const int gapX = GetHorizontalSpacing();
        const int gapY = GetVerticalSpacing();
        auto children = GetVisualChildren();
        auto metrics = GridLayout::ComputeMetrics(
            this,
            effectiveRowDefs,
            effectiveColDefs,
            availableWidth,
            availableHeight,
            children,
            gapX,
            gapY);

        auto refreshedAutoRowHeights = metrics.autoRowHeights;
        auto refreshedAutoColWidths = metrics.autoColWidths;

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
            // skip bad placement
            if (r < 0 || c < 0 || rspan <= 0 || cspan <= 0 ||
                r >= numRows || c >= numCols || rspan > numRows - r || cspan > numCols - c) {
                continue;
            }

            const auto& layout = metrics.childLayouts[i];

            //dont measure a child again if it occupies exactly one auto sized row and one auto‑sized column
            bool rowAuto = (r < static_cast<int>(effectiveRowDefs.size()) && effectiveRowDefs[r].mode == SizeMode::Auto);
            bool colAuto = (c < static_cast<int>(effectiveColDefs.size()) && effectiveColDefs[c].mode == SizeMode::Auto);
            bool singleCell = (rspan == 1 && cspan == 1);
            bool skipMeasure = rowAuto && colAuto && singleCell;

            if (!skipMeasure) {
                const auto margin = fe->GetMargin();
                const int childAvailableWidth = std::max(0, layout.width - margin.Horizontal());
                const int childAvailableHeight = std::max(0, layout.height - margin.Vertical());
                fe->Measure(childAvailableWidth, childAvailableHeight);
            }

            if (rspan == 1 && r < effectiveNumRows && effectiveRowDefs[r].mode == SizeMode::Star && r < static_cast<int>(refreshedAutoRowHeights.size())) {
                const auto margin = fe->GetMargin();
                refreshedAutoRowHeights[r] = std::max(refreshedAutoRowHeights[r], fe->GetDesiredHeight() + margin.Vertical());
            }
            if (cspan == 1 && c < effectiveNumCols && effectiveColDefs[c].mode == SizeMode::Star && c < static_cast<int>(refreshedAutoColWidths.size())) {
                const auto margin = fe->GetMargin();
                refreshedAutoColWidths[c] = std::max(refreshedAutoColWidths[c], fe->GetDesiredWidth() + margin.Horizontal());
            }

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                "[Grid::Measure] Child[%zu] at (%d,%d) span(%d,%d): cell %dx%d, desired %dx%d, horizontal=%d vertical=%d",
                i, r, c, rspan, cspan, layout.width, layout.height,
                fe->GetDesiredWidth(), fe->GetDesiredHeight(),
                static_cast<int>(fe->GetHorizontalAlignment()),
                static_cast<int>(fe->GetVerticalAlignment()));
        }

        m_autoRowHeights = refreshedAutoRowHeights;
        m_autoColWidths = refreshedAutoColWidths;

        int totalW = 0, totalH = 0;
        for (size_t c = 0; c < metrics.colWidths.size(); ++c) {
            const bool isStar = c < effectiveColDefs.size() && effectiveColDefs[c].mode == SizeMode::Star;
            totalW += (isStar && c < refreshedAutoColWidths.size()) ? refreshedAutoColWidths[c] : metrics.colWidths[c];
        }
        for (size_t r = 0; r < metrics.rowHeights.size(); ++r) {
            const bool isStar = r < effectiveRowDefs.size() && effectiveRowDefs[r].mode == SizeMode::Star;
            totalH += (isStar && r < refreshedAutoRowHeights.size()) ? refreshedAutoRowHeights[r] : metrics.rowHeights[r];
        }
        if (metrics.colWidths.size() > 1) {
            totalW += gapX * static_cast<int>(metrics.colWidths.size() - 1);
        }
        if (metrics.rowHeights.size() > 1) {
            totalH += gapY * static_cast<int>(metrics.rowHeights.size() - 1);
        }
        SetDesiredSize(totalW, totalH);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid::Measure] Final desired size: %dx%d", totalW, totalH);
    }

    void Grid::Arrange(int finalWidth, int finalHeight)
    {
        FrameworkElement::Arrange(finalWidth, finalHeight);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid::Arrange] Final size: %dx%d", finalWidth, finalHeight);

        const auto effectiveRowDefs = GridLayout::ResolveDefinitions(m_rowDefs);
        const auto effectiveColDefs = GridLayout::ResolveDefinitions(m_colDefs);
        int numRows = std::max(1, static_cast<int>(effectiveRowDefs.size()));
        int numCols = std::max(1, static_cast<int>(effectiveColDefs.size()));

        const int gapX = GetHorizontalSpacing();
        const int gapY = GetVerticalSpacing();
        auto children = GetVisualChildren();
        auto metrics = GridLayout::ComputeMetrics(
            this,
            effectiveRowDefs,
            effectiveColDefs,
            finalWidth,
            finalHeight,
            children,
            gapX,
            gapY);

        for (size_t i = 0; i < children.size(); ++i) {
            auto child = children[i];
            if (!child) continue;

            int r = GetRow(child);
            int c = GetColumn(child);
            int rspan = GetRowSpan(child);
            int cspan = GetColumnSpan(child);

            if (r < 0 || c < 0 || rspan <= 0 || cspan <= 0 ||
                r >= numRows || c >= numCols || rspan > numRows - r || cspan > numCols - c) {
                continue;
            }

            const auto& layout = metrics.childLayouts[i];
            int x = layout.x;
            int y = layout.y;
            int w = layout.width;
            int h = layout.height;

            auto fe = dynamic_cast<FrameworkElement*>(child);
            if (!fe) continue;


            const auto margin = fe->GetMargin();
            const int contentW = std::max(0, w - margin.Horizontal());
            const int contentH = std::max(0, h - margin.Vertical());
            const int resolvedW = std::max(0, fe->ResolveWidth(contentW, fe->GetDesiredWidth()));
            const int resolvedH = std::max(0, fe->ResolveHeight(contentH, fe->GetDesiredHeight()));

            int alignedX = 0;
            int alignedY = 0;
            int alignedW = resolvedW;
            int alignedH = resolvedH;

            Alignment::ApplyAlignment(
                fe,
                contentW,
                contentH,
                fe->GetHorizontalAlignment(),
                fe->GetVerticalAlignment(),
                alignedX,
                alignedY,
                alignedW,
                alignedH
            );

            if (fe->GetHorizontalAlignment() != HorizontalAlignment::Stretch) {
                alignedW = resolvedW;
            }
            if (fe->GetVerticalAlignment() != VerticalAlignment::Stretch) {
                alignedH = resolvedH;
            }

            fe->Arrange(alignedW, alignedH);

            const int targetX = x + alignedX + margin.Left;
            const int targetY = y + alignedY + margin.Top;
            const int currentX = fe->GetLayoutOffsetX();
            const int currentY = fe->GetLayoutOffsetY();
            if (currentX != targetX || currentY != targetY) {
                fe->SetLayoutOffset(targetX, targetY);
            }

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                "[Grid::Arrange] Child[%zu] at (%d,%d): pos(%d,%d -> %d,%d) size(%dx%d) [cell %dx%d] desired=%dx%d",
                i, r, c, currentX, currentY, targetX, targetY, alignedW, alignedH, w, h,
                resolvedW, resolvedH);
        }
    }

}