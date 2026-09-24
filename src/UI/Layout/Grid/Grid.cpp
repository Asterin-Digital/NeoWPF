#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/GridLayout.h>
#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
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

    void Grid::AddRow(SizeMode mode, int value)
    {
        auto rowDefinition = std::make_shared<RowDefinition>();
        rowDefinition->SetHeight({mode, value});
        AddRowDefinition(std::move(rowDefinition));
    }

    void Grid::AddRows(std::initializer_list<Definition> defs)
    {
        for (const auto& definition : defs) {
            AddRow(definition.mode, definition.value);
        }
    }

    void Grid::AddColumn(SizeMode mode, int value)
    {
        auto columnDefinition = std::make_shared<ColumnDefinition>();
        columnDefinition->SetWidth({mode, value});
        AddColumnDefinition(std::move(columnDefinition));
    }

    void Grid::AddColumns(std::initializer_list<Definition> defs)
    {
        for (const auto& definition : defs) {
            AddColumn(definition.mode, definition.value);
        }
    }

    void Grid::SetRow(Visual* v, int r) noexcept
    {
        if (!v) return;
        if (auto* obj = dynamic_cast<NeoWPF::UI::Properties::DependencyObject*>(v)) {
            if (r <= 0) {
                obj->ClearValue(Grid::RowProperty());
            } else {
                obj->SetValue(Grid::RowProperty(), r);
            }
        }
    }

    void Grid::SetColumn(Visual* v, int c) noexcept
    {
        if (!v) return;
        if (auto* obj = dynamic_cast<NeoWPF::UI::Properties::DependencyObject*>(v)) {
            if (c <= 0) {
                obj->ClearValue(Grid::ColumnProperty());
            } else {
                obj->SetValue(Grid::ColumnProperty(), c);
            }
        }
    }

    int Grid::GetRow(Visual* v) const noexcept
    {
        if (!v) return 0;
        if (auto* obj = dynamic_cast<const NeoWPF::UI::Properties::DependencyObject*>(v)) {
            try {
                return obj->GetValue<int>(Grid::RowProperty());
            } catch (const std::bad_any_cast&) {
            }
        }
        return 0;
    }

    int Grid::GetColumn(Visual* v) const noexcept
    {
        if (!v) return 0;
        if (auto* obj = dynamic_cast<const NeoWPF::UI::Properties::DependencyObject*>(v)) {
            try {
                return obj->GetValue<int>(Grid::ColumnProperty());
            } catch (const std::bad_any_cast&) {
            }
        }
        return 0;
    }

    void Grid::ClearAttachedProperties(Visual* v)
    {
        if (!v) return;
        if (auto* obj = dynamic_cast<NeoWPF::UI::Properties::DependencyObject*>(v)) {
            obj->ClearValue(Grid::RowProperty());
            obj->ClearValue(Grid::ColumnProperty());
            obj->ClearValue(Grid::RowSpanProperty());
            obj->ClearValue(Grid::ColumnSpanProperty());
        }
    }

    void Grid::AddRowDefinition(std::shared_ptr<RowDefinition> rowDef)
    {
        if (!rowDef) return;

        const auto height = rowDef->GetHeight();
        m_rowDefs.push_back({height.Mode, height.Value});
    }

    void Grid::AddColumnDefinition(std::shared_ptr<ColumnDefinition> colDef)
    {
        if (!colDef) return;

        const auto width = colDef->GetWidth();
        m_colDefs.push_back({width.Mode, width.Value});
    }

    void Grid::OnRemoveVisualChild(Visual* child)
    {
        ClearAttachedProperties(child);
    }

    void Grid::SetRowSpan(Visual* v, int span)
    {
        if (!v) return;
        if (auto* obj = dynamic_cast<NeoWPF::UI::Properties::DependencyObject*>(v)) {
            if (span <= 1) {
                obj->ClearValue(Grid::RowSpanProperty());
            } else {
                obj->SetValue(Grid::RowSpanProperty(), span);
            }
        }
    }

    void Grid::SetColumnSpan(Visual* v, int span)
    {
        if (!v) return;
        if (auto* obj = dynamic_cast<NeoWPF::UI::Properties::DependencyObject*>(v)) {
            if (span <= 1) {
                obj->ClearValue(Grid::ColumnSpanProperty());
            } else {
                obj->SetValue(Grid::ColumnSpanProperty(), span);
            }
        }
    }

    int Grid::GetRowSpan(Visual* v)
    {
        if (!v) return 1;
        if (auto* obj = dynamic_cast<const NeoWPF::UI::Properties::DependencyObject*>(v)) {
            try {
                return obj->GetValue<int>(Grid::RowSpanProperty());
            } catch (const std::bad_any_cast&) {
            }
        }
        return 1;
    }

    int Grid::GetColumnSpan(Visual* v)
    {
        if (!v) return 1;
        if (auto* obj = dynamic_cast<const NeoWPF::UI::Properties::DependencyObject*>(v)) {
            try {
                return obj->GetValue<int>(Grid::ColumnSpanProperty());
            } catch (const std::bad_any_cast&) {
            }
        }
        return 1;
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

        if (child.get() == this) {
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

    void Grid::Measure(double availableWidth, double availableHeight)
    {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid::Measure] Available: %gx%g", availableWidth, availableHeight);

        const auto padding = GetPadding();
        const double contentAvailableWidth = (std::max)(0.0, availableWidth - padding.Horizontal());
        const double contentAvailableHeight = (std::max)(0.0, availableHeight - padding.Vertical());

        const int INF = 1000000;
        const std::size_t numRows = m_rowDefs.empty() ? 1u : m_rowDefs.size();
        const std::size_t numCols = m_colDefs.empty() ? 1u : m_colDefs.size();

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
            const std::size_t effectiveNumRows = effectiveRowDefs.empty() ? 1u : effectiveRowDefs.size();
            const std::size_t effectiveNumCols = effectiveColDefs.empty() ? 1u : effectiveColDefs.size();

            bool measureWidth = (cspan == 1 && c < effectiveNumCols && c < effectiveColDefs.size() && effectiveColDefs[c].mode == SizeMode::Auto);
            bool measureHeight = (rspan == 1 && r < effectiveNumRows && r < effectiveRowDefs.size() && effectiveRowDefs[r].mode == SizeMode::Auto);

            if (measureWidth || measureHeight)
                fe->Measure(measureWidth ? INF : contentAvailableWidth,
                            measureHeight ? INF : contentAvailableHeight);
        }

        const auto effectiveRowDefs = GridLayout::ResolveDefinitions(m_rowDefs);
        const auto effectiveColDefs = GridLayout::ResolveDefinitions(m_colDefs);
        const std::size_t effectiveNumRows = effectiveRowDefs.empty() ? 1u : effectiveRowDefs.size();
        const std::size_t effectiveNumCols = effectiveColDefs.empty() ? 1u : effectiveColDefs.size();
        const double gapX = GetHorizontalSpacing();
        const double gapY = GetVerticalSpacing();
        auto children = GetVisualChildren();
        auto metrics = GridLayout::ComputeMetrics(
            this,
            effectiveRowDefs,
            effectiveColDefs,
            contentAvailableWidth,
            contentAvailableHeight,
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
            bool rowAuto = (r < effectiveRowDefs.size() && effectiveRowDefs[r].mode == SizeMode::Auto);
            bool colAuto = (c < effectiveColDefs.size() && effectiveColDefs[c].mode == SizeMode::Auto);
            bool singleCell = (rspan == 1 && cspan == 1);
            bool skipMeasure = rowAuto && colAuto && singleCell;

            if (!skipMeasure) {
                const auto margin = fe->GetMargin();
                const double childAvailableWidth = layout.width > margin.Horizontal() ? layout.width - margin.Horizontal() : 0.0;
                const double childAvailableHeight = layout.height > margin.Vertical() ? layout.height - margin.Vertical() : 0.0;
                fe->Measure(childAvailableWidth, childAvailableHeight);
            }

            if (rspan == 1 && r < effectiveNumRows && effectiveRowDefs[r].mode == SizeMode::Star && r < refreshedAutoRowHeights.size()) {
                const auto margin = fe->GetMargin();
                const double candidateHeight = fe->GetDesiredHeight() + margin.Vertical();
                refreshedAutoRowHeights[r] = candidateHeight > refreshedAutoRowHeights[r] ? candidateHeight : refreshedAutoRowHeights[r];
            }
            if (cspan == 1 && c < effectiveNumCols && effectiveColDefs[c].mode == SizeMode::Star && c < refreshedAutoColWidths.size()) {
                const auto margin = fe->GetMargin();
                const double candidateWidth = fe->GetDesiredWidth() + margin.Horizontal();
                refreshedAutoColWidths[c] = candidateWidth > refreshedAutoColWidths[c] ? candidateWidth : refreshedAutoColWidths[c];
            }

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                "[Grid::Measure] Child[%zu] at (%d,%d) span(%d,%d): cell %gx%g, desired %gx%g, horizontal=%d vertical=%d",
                i, r, c, rspan, cspan, layout.width, layout.height,
                fe->GetDesiredWidth(), fe->GetDesiredHeight(),
                fe->GetHorizontalAlignment(),
                fe->GetVerticalAlignment());
        }

        m_autoRowHeights = refreshedAutoRowHeights;
        m_autoColWidths = refreshedAutoColWidths;

        double totalW = 0.0, totalH = 0.0;
        for (size_t c = 0; c < metrics.colWidths.size(); ++c) {
            const bool isStar = c < effectiveColDefs.size() && effectiveColDefs[c].mode == SizeMode::Star;
            totalW += (isStar && c < refreshedAutoColWidths.size()) ? refreshedAutoColWidths[c] : metrics.colWidths[c];
        }
        for (size_t r = 0; r < metrics.rowHeights.size(); ++r) {
            const bool isStar = r < effectiveRowDefs.size() && effectiveRowDefs[r].mode == SizeMode::Star;
            totalH += (isStar && r < refreshedAutoRowHeights.size()) ? refreshedAutoRowHeights[r] : metrics.rowHeights[r];
        }
        if (metrics.colWidths.size() > 1) {
            totalW += gapX * (metrics.colWidths.size() > 1 ? metrics.colWidths.size() - 1u : 0.0);
        }
        if (metrics.rowHeights.size() > 1) {
            totalH += gapY * (metrics.rowHeights.size() > 1 ? metrics.rowHeights.size() - 1u : 0.0);
        }
        SetDesiredSize(totalW + padding.Horizontal(), totalH + padding.Vertical());
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid::Measure] Final desired size: %gx%g", totalW, totalH);
    }

 /*   Grid::Arrange (read below)
 *    Final Width × Final Height
 *              │
 *              ▼
 *    Resolve row / column definitions
 *              │
 *              ├── no row definitions  ──────────►  1 row
 *              │
 *              ├── no column definitions ────────►  1 column
 *              │
 *              ▼
 *       Compute grid metrics
 *              │
 *              ▼
 *       For each child
 *              │
 *              ▼
 *       child == null?
 *              │
 *        ┌─────┴───────────────► YES ──► SKIP
 *        │
 *       NO
 *        │
 *        ▼
 *    Read row / column
 *    Read rowSpan / colSpan
 *        │
 *        ▼
 *    Is grid position valid?
 *        │
 *        ├── NO ───────────────────────────────► SKIP
 *        │
 *       YES
 *        │
 *        ▼
 *    Get computed child layout
 *    (x, y, width, height)
 *        │
 *        ▼
 *    FrameworkElement available?
 *        │
 *        ├── NO ───────────────────────────────► SKIP
 *        │
 *       YES
 *        │
 *        ▼
 *    Read margin
 *        │
 *        ▼
 *    Calculate content area
 *        │
 *        ▼
 *    Resolve requested width / height
 *        │
 *        ▼
 *    Apply alignment
 *        │
 *        ├──────────── Horizontal ──────────────┐
 *        │                                      │
 *        │    Stretch ──► aligned width        │
 *        │                                      │
 *        │    Other   ──► resolved width       │
 *        │                                      │
 *        └───────────────┬──────────────────────┘
 *                        │
 *                        ├──────────── Vertical ───────────────┐
 *                        │                                     │
 *                        │    Stretch ──► aligned height      │
 *                        │                                     │
 *                        │    Other   ──► resolved height     │
 *                        │                                     │
 *                        └──────────────────┬──────────────────┘
 *                                           │
 *                                           ▼
 *                                   Arrange the child
 *                                           │
 *                                           ▼
 *                                   Calculate target offset
 *                                           │
 *                                           ▼
 *                                   Current offset == target?
 *                                           │
 *                                    ┌──────┴──────┐
 *                                    │             │
 *                                   YES            NO
 *                                    │             │
 *                                    ▼             ▼
 *                                  No-op     Update layout offset
 *                                                  │
 *                                                  ▼
 *                                              Next child
 */
    void Grid::Arrange(double finalWidth, double finalHeight)
    {
        FrameworkElement::Arrange(finalWidth, finalHeight);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Info,
            "[Grid::Arrange] Final size: %gx%g", finalWidth, finalHeight);

        const auto effectiveRowDefs = GridLayout::ResolveDefinitions(m_rowDefs);
        const auto effectiveColDefs = GridLayout::ResolveDefinitions(m_colDefs);
        const std::size_t numRows = effectiveRowDefs.empty() ? 1u : effectiveRowDefs.size();
        const std::size_t numCols = effectiveColDefs.empty() ? 1u : effectiveColDefs.size();

        const double gapX = GetHorizontalSpacing();
        const double gapY = GetVerticalSpacing();
        const auto padding = GetPadding();
        const double contentWidth = (std::max)(0.0, finalWidth - padding.Horizontal());
        const double contentHeight = (std::max)(0.0, finalHeight - padding.Vertical());
        auto children = GetVisualChildren();
        auto metrics = GridLayout::ComputeMetrics(
            this,
            effectiveRowDefs,
            effectiveColDefs,
            contentWidth,
            contentHeight,
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
            double x = layout.x;
            double y = layout.y;
            double w = layout.width;
            double h = layout.height;

            auto fe = dynamic_cast<FrameworkElement*>(child);
            if (!fe) continue;


            const auto margin = fe->GetMargin();
            const double contentW = (std::max)(0.0, w - margin.Horizontal());
            const double contentH = (std::max)(0.0, h - margin.Vertical());
            const double resolvedW = (std::max)(0.0, fe->ResolveWidth(contentW, fe->GetDesiredWidth()));
            const double resolvedH = (std::max)(0.0, fe->ResolveHeight(contentH, fe->GetDesiredHeight()));

            double alignedX = 0.0;
            double alignedY = 0.0;
            double alignedW = resolvedW;
            double alignedH = resolvedH;

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
                alignedW = resolvedW;            }
            if (fe->GetVerticalAlignment() != VerticalAlignment::Stretch) {
                alignedH = resolvedH;            }

            fe->Arrange(alignedW, alignedH);

            const double targetX = padding.Left + x + alignedX + margin.Left;
            const double targetY = padding.Top + y + alignedY + margin.Top;
            const double currentX = fe->GetLayoutOffsetX();
            const double currentY = fe->GetLayoutOffsetY();
            if (currentX != targetX || currentY != targetY) {
                fe->SetLayoutOffset(targetX, targetY);
            }

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::GridLayout, NeoWPF::Diagnostics::Level::Debug,
                "[Grid::Arrange] Child[%zu] at (%d,%d): pos(%g,%g -> %g,%g) size(%gx%g) [cell %gx%g] desired=%gx%g",
                i, r, c, currentX, currentY, targetX, targetY, alignedW, alignedH, w, h,
                resolvedW, resolvedH);
        }
    }

}