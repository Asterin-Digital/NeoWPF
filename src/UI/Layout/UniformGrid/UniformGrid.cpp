#include <NeoWPF/UI/Layout/UniformGrid/UniformGrid.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace {

using NeoWPF::Diagnostics::DeveloperWarningSeverity;
using NeoWPF::Diagnostics::DeveloperWarnings;

struct UniformGridDimensions {
    std::size_t rows = 1;
    std::size_t columns = 1;
    std::size_t firstColumn = 0;
};

std::size_t AddCapacity(std::size_t childCount, std::size_t firstColumn)
{
    if (childCount > (std::numeric_limits<std::size_t>::max)() - firstColumn) {
        return (std::numeric_limits<std::size_t>::max)();
    }
    return childCount + firstColumn;
}

std::size_t DivideRoundUp(std::size_t value, std::size_t divisor)
{
    return value / divisor + (value % divisor != 0 ? 1u : 0u);
}

void QueueUniformGridDiagnostic(const wchar_t* code,
                               const wchar_t* summary,
                               const std::wstring& detail,
                               DeveloperWarningSeverity severity)
{
    NeoWPF::Diagnostics::QueueDeveloperWarning(
        std::wstring(code) + L": " + summary,
        detail,
        severity);
}

UniformGridDimensions ComputeDimensions(const NeoWPF::UI::UniformGrid& grid,
                                        std::size_t childCount)
{
    int configuredRows = grid.GetRows();
    int configuredColumns = grid.GetColumns();
    const int requestedFirstColumn = grid.GetFirstColumn();

    if (configuredRows < 0) {
        QueueUniformGridDiagnostic(
            DeveloperWarnings::kUniformGridInvalidDimensionCode,
            L"UniformGrid Rows is negative",
            L"Negative Rows is treated as automatic sizing.",
            DeveloperWarningSeverity::Error);
        configuredRows = 0;
    }
    if (configuredColumns < 0) {
        QueueUniformGridDiagnostic(
            DeveloperWarnings::kUniformGridInvalidDimensionCode,
            L"UniformGrid Columns is negative",
            L"Negative Columns is treated as automatic sizing.",
            DeveloperWarningSeverity::Error);
        configuredColumns = 0;
    }

    const std::size_t nonnegativeFirstColumn = requestedFirstColumn > 0
        ? static_cast<std::size_t>(requestedFirstColumn)
        : 0;
    const std::size_t capacityForColumns = AddCapacity(childCount, nonnegativeFirstColumn);
    UniformGridDimensions dimensions;

    if (configuredRows == 0 && configuredColumns == 0) {
        dimensions.columns = (std::max)(
            std::size_t{1},
            static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<double>(capacityForColumns)))));
    } else if (configuredColumns == 0) {
        dimensions.rows = static_cast<std::size_t>(configuredRows);
        dimensions.columns = (std::max)(
            std::size_t{1},
            DivideRoundUp(capacityForColumns, dimensions.rows));
    } else {
        dimensions.columns = static_cast<std::size_t>(configuredColumns);
    }

    if (requestedFirstColumn < 0) {
        QueueUniformGridDiagnostic(
            DeveloperWarnings::kUniformGridInvalidFirstColumnCode,
            L"UniformGrid FirstColumn is negative",
            L"A negative FirstColumn is clamped to zero.",
            DeveloperWarningSeverity::Error);
    } else if (nonnegativeFirstColumn >= dimensions.columns) {
        QueueUniformGridDiagnostic(
            DeveloperWarnings::kUniformGridInvalidFirstColumnCode,
            L"UniformGrid FirstColumn exceeds the effective columns",
            L"FirstColumn is clamped to the last effective column.",
            DeveloperWarningSeverity::Error);
    }

    dimensions.firstColumn = requestedFirstColumn < 0
        ? 0
        : (std::min)(nonnegativeFirstColumn, dimensions.columns - 1);
    const std::size_t capacityForRows = AddCapacity(childCount, dimensions.firstColumn);

    if (configuredRows == 0) {
        dimensions.rows = (std::max)(
            std::size_t{1},
            DivideRoundUp(capacityForRows, dimensions.columns));
    } else if (configuredColumns > 0) {
        const std::size_t requiredRows = (std::max)(
            std::size_t{1},
            DivideRoundUp(capacityForRows, dimensions.columns));
        dimensions.rows = (std::max)(static_cast<std::size_t>(configuredRows), requiredRows);
        if (requiredRows > static_cast<std::size_t>(configuredRows)) {
            QueueUniformGridDiagnostic(
                DeveloperWarnings::kUniformGridCapacityOverflowCode,
                L"UniformGrid capacity exceeded",
                L"The effective row count was increased to preserve all visible children.",
                DeveloperWarningSeverity::Warning);
        }
    }

    return dimensions;
}

std::vector<NeoWPF::UI::FrameworkElement*> GetParticipatingChildren(NeoWPF::UI::UniformGrid& grid)
{
    std::vector<NeoWPF::UI::FrameworkElement*> children;
    for (auto* child : grid.GetVisualChildren()) {
        if (auto* uiChild = dynamic_cast<NeoWPF::UI::UIElement*>(child); uiChild && !uiChild->IsVisible()) {
            continue;
        }
        if (auto* frameworkChild = dynamic_cast<NeoWPF::UI::FrameworkElement*>(child)) {
            children.push_back(frameworkChild);
        }
    }
    return children;
}

}

namespace NeoWPF::UI {

UniformGrid::UniformGrid()
{
}

int UniformGrid::GetRows() const
{
    return GetValue<int>(RowsProperty());
}

void UniformGrid::SetRows(int rows)
{
    SetValue(RowsProperty(), rows);
}

int UniformGrid::GetColumns() const
{
    return GetValue<int>(ColumnsProperty());
}

void UniformGrid::SetColumns(int columns)
{
    SetValue(ColumnsProperty(), columns);
}

int UniformGrid::GetFirstColumn() const
{
    return GetValue<int>(FirstColumnProperty());
}

void UniformGrid::SetFirstColumn(int firstColumn)
{
    SetValue(FirstColumnProperty(), firstColumn);
}

void UniformGrid::Measure(double availableWidth, double availableHeight)
{
    const auto children = GetParticipatingChildren(*this);
    const auto dimensions = ComputeDimensions(*this, children.size());
    const double cellAvailableWidth = (std::max)(
        0.0,
        availableWidth / static_cast<double>(dimensions.columns));
    const double cellAvailableHeight = (std::max)(
        0.0,
        availableHeight / static_cast<double>(dimensions.rows));
    double cellDesiredWidth = 0.0;
    double cellDesiredHeight = 0.0;

    for (auto* child : children) {
        const auto margin = child->GetMargin();
        const double childAvailableWidth = (std::max)(0.0, cellAvailableWidth - margin.Horizontal());
        const double childAvailableHeight = (std::max)(0.0, cellAvailableHeight - margin.Vertical());
        child->Measure(childAvailableWidth, childAvailableHeight);

        const double desiredWidth = (std::max)(
            0.0,
            child->ResolveWidth(childAvailableWidth, child->GetDesiredWidth()));
        const double desiredHeight = (std::max)(
            0.0,
            child->ResolveHeight(childAvailableHeight, child->GetDesiredHeight()));
        child->SetDesiredSize(desiredWidth, desiredHeight);
        cellDesiredWidth = (std::max)(cellDesiredWidth, desiredWidth + margin.Horizontal());
        cellDesiredHeight = (std::max)(cellDesiredHeight, desiredHeight + margin.Vertical());
    }

    SetDesiredSize(
        ResolveWidth(availableWidth, cellDesiredWidth * static_cast<double>(dimensions.columns)),
        ResolveHeight(availableHeight, cellDesiredHeight * static_cast<double>(dimensions.rows)));
}

void UniformGrid::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    const auto children = GetParticipatingChildren(*this);
    const auto dimensions = ComputeDimensions(*this, children.size());
    const double cellWidth = (std::max)(0.0, GetFinalWidth() / static_cast<double>(dimensions.columns));
    const double cellHeight = (std::max)(0.0, GetFinalHeight() / static_cast<double>(dimensions.rows));

    for (std::size_t index = 0; index < children.size(); ++index) {
        auto* child = children[index];
        const std::size_t cellIndex = index + dimensions.firstColumn;
        const std::size_t row = cellIndex / dimensions.columns;
        const std::size_t column = cellIndex % dimensions.columns;
        const auto margin = child->GetMargin();
        const double contentWidth = (std::max)(0.0, cellWidth - margin.Horizontal());
        const double contentHeight = (std::max)(0.0, cellHeight - margin.Vertical());
        const double resolvedWidth = (std::max)(0.0, child->ResolveWidth(contentWidth, child->GetDesiredWidth()));
        const double resolvedHeight = (std::max)(0.0, child->ResolveHeight(contentHeight, child->GetDesiredHeight()));

        double alignedX = 0.0;
        double alignedY = 0.0;
        double alignedWidth = resolvedWidth;
        double alignedHeight = resolvedHeight;
        Alignment::ApplyAlignment(
            child,
            contentWidth,
            contentHeight,
            child->GetHorizontalAlignment(),
            child->GetVerticalAlignment(),
            alignedX,
            alignedY,
            alignedWidth,
            alignedHeight);

        if (child->GetHorizontalAlignment() != HorizontalAlignment::Stretch) {
            alignedWidth = resolvedWidth;
        }
        if (child->GetVerticalAlignment() != VerticalAlignment::Stretch) {
            alignedHeight = resolvedHeight;
        }

        child->Arrange(alignedWidth, alignedHeight);
        child->SetLayoutOffset(
            static_cast<double>(column) * cellWidth + margin.Left + alignedX,
            static_cast<double>(row) * cellHeight + margin.Top + alignedY);
    }
}

}