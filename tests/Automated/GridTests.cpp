#include <gtest/gtest.h>

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>

#include <limits>
#include <memory>
#include <vector>

namespace {

class FixedGridChild final : public NeoWPF::UI::FrameworkElement {
public:
    FixedGridChild(int width, int height) : width(width), height(height)
    {
        SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Stretch);
        SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Stretch);
    }

    void Measure(double, double) override
    {
        SetDesiredSize(width, height);
    }

private:
    int width;
    int height;
};

struct GridPlacementSpec {
    int row = 0;
    int column = 0;
    int rowSpan = 1;
    int columnSpan = 1;
    int width = 1;
    int height = 1;
};

void ExpectChildStaysInsideGrid(const std::shared_ptr<FixedGridChild>& child, int gridWidth, int gridHeight)
{
    ASSERT_TRUE(child != nullptr);
    EXPECT_GE(child->GetFinalWidth(), 0);
    EXPECT_GE(child->GetFinalHeight(), 0);
    EXPECT_GE(child->GetLayoutOffsetX(), 0);
    EXPECT_GE(child->GetLayoutOffsetY(), 0);
    EXPECT_LE(child->GetLayoutOffsetX() + child->GetFinalWidth(), gridWidth);
    EXPECT_LE(child->GetLayoutOffsetY() + child->GetFinalHeight(), gridHeight);
}

TEST(GridTests, ArrangesChildrenAcrossFixedAndStarCells)
{
    NeoWPF::UI::Grid grid;
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 20);
    grid.AddRow(NeoWPF::UI::SizeMode::Star, 1);
    grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, 30);
    grid.AddColumn(NeoWPF::UI::SizeMode::Star, 1);

    auto first = std::make_shared<FixedGridChild>(10, 10);
    auto second = std::make_shared<FixedGridChild>(10, 10);
    grid.AddChild(first, 0, 0);
    grid.AddChild(second, 1, 1);

    grid.Measure(100, 80);
    grid.Arrange(100, 80);

    EXPECT_EQ(first->GetLayoutOffsetX(), 0);
    EXPECT_EQ(first->GetLayoutOffsetY(), 0);
    EXPECT_EQ(first->GetFinalWidth(), 30);
    EXPECT_EQ(first->GetFinalHeight(), 20);
    EXPECT_EQ(second->GetLayoutOffsetX(), 30);
    EXPECT_EQ(second->GetLayoutOffsetY(), 20);
    EXPECT_EQ(second->GetFinalWidth(), 70);
    EXPECT_EQ(second->GetFinalHeight(), 60);
}

TEST(GridTests, ArrangesSpanningChildAcrossMultipleCells)
{
    NeoWPF::UI::Grid grid;
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 20);
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 30);
    grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, 40);
    grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, 60);

    auto spanning = std::make_shared<FixedGridChild>(10, 10);
    auto cell = std::make_shared<FixedGridChild>(10, 10);
    grid.AddChild(spanning, 0, 0, 1, 2);
    grid.AddChild(cell, 1, 1);

    grid.Measure(100, 50);
    grid.Arrange(100, 50);

    EXPECT_EQ(spanning->GetLayoutOffsetX(), 0);
    EXPECT_EQ(spanning->GetLayoutOffsetY(), 0);
    EXPECT_EQ(spanning->GetFinalWidth(), 100);
    EXPECT_EQ(spanning->GetFinalHeight(), 20);
    EXPECT_EQ(cell->GetLayoutOffsetX(), 40);
    EXPECT_EQ(cell->GetLayoutOffsetY(), 20);
    EXPECT_EQ(cell->GetFinalWidth(), 60);
    EXPECT_EQ(cell->GetFinalHeight(), 30);
}

TEST(GridTests, AutoTracksUseLargestChildSizeIncludingMargins)
{
    NeoWPF::UI::Grid grid;
    grid.SetSpacing(NeoWPF::UI::Spacing(6, 0));
    grid.AddRow(NeoWPF::UI::SizeMode::Auto);
    grid.AddColumn(NeoWPF::UI::SizeMode::Auto);
    grid.AddColumn(NeoWPF::UI::SizeMode::Auto);

    auto first = std::make_shared<FixedGridChild>(20, 10);
    first->SetMargin(2, 1, 3, 2);
    auto second = std::make_shared<FixedGridChild>(35, 14);
    second->SetMargin(4, 0, 1, 0);
    grid.AddChild(first, 0, 0);
    grid.AddChild(second, 0, 1);

    grid.Measure(200, 100);
    grid.Arrange(71, 14);

    EXPECT_DOUBLE_EQ(grid.GetDesiredWidth(), 71.0);
    EXPECT_DOUBLE_EQ(grid.GetDesiredHeight(), 14.0);
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetX(), 2.0);
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetY(), 1.0);
    EXPECT_DOUBLE_EQ(first->GetFinalWidth(), 20.0);
    EXPECT_DOUBLE_EQ(first->GetFinalHeight(), 11.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetX(), 35.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetY(), 0.0);
    EXPECT_DOUBLE_EQ(second->GetFinalWidth(), 35.0);
    EXPECT_DOUBLE_EQ(second->GetFinalHeight(), 14.0);
}

TEST(GridTests, WeightedStarTracksReallocateRemainingSpaceWhenResized)
{
    NeoWPF::UI::Grid grid;
    grid.SetSpacing(NeoWPF::UI::Spacing(5, 0));
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 20);
    grid.AddColumn(NeoWPF::UI::SizeMode::Auto);
    grid.AddColumn(NeoWPF::UI::SizeMode::Star, 1);
    grid.AddColumn(NeoWPF::UI::SizeMode::Star, 2);

    auto autoChild = std::make_shared<FixedGridChild>(30, 10);
    auto firstStarChild = std::make_shared<FixedGridChild>(10, 10);
    auto secondStarChild = std::make_shared<FixedGridChild>(20, 10);
    grid.AddChild(autoChild, 0, 0);
    grid.AddChild(firstStarChild, 0, 1);
    grid.AddChild(secondStarChild, 0, 2);

    grid.Measure(160, 20);
    grid.Arrange(160, 20);

    EXPECT_DOUBLE_EQ(autoChild->GetFinalWidth(), 30.0);
    EXPECT_DOUBLE_EQ(firstStarChild->GetLayoutOffsetX(), 35.0);
    EXPECT_DOUBLE_EQ(firstStarChild->GetFinalWidth(), 40.0);
    EXPECT_DOUBLE_EQ(secondStarChild->GetLayoutOffsetX(), 80.0);
    EXPECT_DOUBLE_EQ(secondStarChild->GetFinalWidth(), 80.0);

    grid.Measure(190, 20);
    grid.Arrange(190, 20);

    EXPECT_DOUBLE_EQ(firstStarChild->GetLayoutOffsetX(), 35.0);
    EXPECT_DOUBLE_EQ(firstStarChild->GetFinalWidth(), 50.0);
    EXPECT_DOUBLE_EQ(secondStarChild->GetLayoutOffsetX(), 90.0);
    EXPECT_DOUBLE_EQ(secondStarChild->GetFinalWidth(), 100.0);
}

TEST(GridTests, AppliesHorizontalAndVerticalSpacingBetweenCells)
{
    NeoWPF::UI::Grid grid;
    grid.SetSpacing(NeoWPF::UI::Spacing(5, 7));
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 20);
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 20);
    grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, 30);
    grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, 30);

    auto child = std::make_shared<FixedGridChild>(10, 10);
    grid.AddChild(child, 1, 1);

    grid.Measure(65, 47);
    grid.Arrange(65, 47);

    EXPECT_EQ(child->GetLayoutOffsetX(), 35);
    EXPECT_EQ(child->GetLayoutOffsetY(), 27);
    EXPECT_EQ(child->GetFinalWidth(), 30);
    EXPECT_EQ(child->GetFinalHeight(), 20);
}

TEST(GridTests, ArrangesNestedGridWithinParentCell)
{
    NeoWPF::UI::Grid parent;
    parent.AddRow(NeoWPF::UI::SizeMode::Fixed, 50);
    parent.AddColumn(NeoWPF::UI::SizeMode::Fixed, 100);

    auto nested = std::make_shared<NeoWPF::UI::Grid>();
    nested->AddRow(NeoWPF::UI::SizeMode::Fixed, 20);
    nested->AddColumn(NeoWPF::UI::SizeMode::Fixed, 30);
    nested->AddColumn(NeoWPF::UI::SizeMode::Star, 1);
    auto nestedChild = std::make_shared<FixedGridChild>(10, 10);
    nested->AddChild(nestedChild, 0, 1);
    parent.AddChild(nested, 0, 0);

    parent.Measure(100, 50);
    parent.Arrange(100, 50);

    EXPECT_EQ(nested->GetLayoutOffsetX(), 0);
    EXPECT_EQ(nested->GetLayoutOffsetY(), 0);
    EXPECT_EQ(nested->GetFinalWidth(), 100);
    EXPECT_EQ(nested->GetFinalHeight(), 50);
    EXPECT_EQ(nestedChild->GetLayoutOffsetX(), 30);
    EXPECT_EQ(nestedChild->GetLayoutOffsetY(), 0);
    EXPECT_EQ(nestedChild->GetFinalWidth(), 70);
    EXPECT_EQ(nestedChild->GetFinalHeight(), 20);
}

TEST(GridTests, SkipsChildrenWithInvalidPlacement)
{
    NeoWPF::UI::Grid grid;
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 20);
    grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, 20);

    grid.AddChild(nullptr, 0, 0);
    std::vector<std::shared_ptr<FixedGridChild>> invalidChildren;
    invalidChildren.push_back(std::make_shared<FixedGridChild>(10, 10));
    invalidChildren.push_back(std::make_shared<FixedGridChild>(10, 10));
    invalidChildren.push_back(std::make_shared<FixedGridChild>(10, 10));
    invalidChildren.push_back(std::make_shared<FixedGridChild>(10, 10));
    invalidChildren.push_back(std::make_shared<FixedGridChild>(10, 10));
    invalidChildren.push_back(std::make_shared<FixedGridChild>(10, 10));
    grid.AddChild(invalidChildren[0], -1, 0);
    grid.AddChild(invalidChildren[1], 0, -1);
    grid.AddChild(invalidChildren[2], 2, 0);
    grid.AddChild(invalidChildren[3], 0, 2);
    grid.AddChild(invalidChildren[4], 0, 0, 0, 1);
    grid.AddChild(invalidChildren[5], 0, 0, 1, 0);
    grid.AddChild(std::shared_ptr<NeoWPF::UI::Visual>(&grid, [](NeoWPF::UI::Visual*) {}), 0, 0);

    grid.Measure(100, 100);
    grid.Arrange(100, 100);

    EXPECT_EQ(grid.GetVisualChildren().size(), invalidChildren.size() - 4);
    for (const auto& invalid : invalidChildren) {
        EXPECT_EQ(invalid->GetFinalWidth(), 0);
        EXPECT_EQ(invalid->GetFinalHeight(), 0);
    }
}

TEST(GridTests, DistributesWeightedStarSpaceAcrossRowsAndColumns)
{
    NeoWPF::UI::Grid grid;
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 10);
    grid.AddRow(NeoWPF::UI::SizeMode::Star, 1);
    grid.AddRow(NeoWPF::UI::SizeMode::Star, 2);
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 15);
    grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, 20);
    grid.AddColumn(NeoWPF::UI::SizeMode::Star, 1);
    grid.AddColumn(NeoWPF::UI::SizeMode::Star, 2);
    grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, 10);
    grid.SetSpacing(NeoWPF::UI::Spacing(5, 5));

    auto topLeft = std::make_shared<FixedGridChild>(10, 10);
    auto midRight = std::make_shared<FixedGridChild>(10, 10);
    grid.AddChild(topLeft, 0, 0);
    grid.AddChild(midRight, 1, 2);

    grid.Measure(200, 200);
    grid.Arrange(200, 200);

    EXPECT_EQ(topLeft->GetLayoutOffsetX(), 0);
    EXPECT_EQ(topLeft->GetLayoutOffsetY(), 0);
    EXPECT_EQ(topLeft->GetFinalWidth(), 20);
    EXPECT_EQ(topLeft->GetFinalHeight(), 10);
    EXPECT_EQ(midRight->GetLayoutOffsetX(), 81);
    EXPECT_EQ(midRight->GetLayoutOffsetY(), 15);
    EXPECT_EQ(midRight->GetFinalWidth(), 102);
    EXPECT_EQ(midRight->GetFinalHeight(), 53);
}

TEST(GridTests, KeepsChildrenInsideBoundsAcrossManyMixedLayouts)
{
    struct LayoutCase {
        std::vector<NeoWPF::UI::Grid::Definition> rows;
        std::vector<NeoWPF::UI::Grid::Definition> columns;
        std::vector<GridPlacementSpec> placements;
        int width = 0;
        int height = 0;
        int spacingX = 0;
        int spacingY = 0;
    };

    const std::vector<LayoutCase> cases = {
        {
            {{NeoWPF::UI::SizeMode::Fixed, 20}, {NeoWPF::UI::SizeMode::Auto, 0}, {NeoWPF::UI::SizeMode::Star, 1}},
            {{NeoWPF::UI::SizeMode::Fixed, 30}, {NeoWPF::UI::SizeMode::Star, 1}, {NeoWPF::UI::SizeMode::Fixed, 40}},
            {{0, 0, 1, 1, 10, 10}, {1, 1, 1, 1, 15, 20}, {2, 2, 1, 1, 5, 15}},
            200, 160, 5, 7
        },
        {
            {{NeoWPF::UI::SizeMode::Star, 2}, {NeoWPF::UI::SizeMode::Fixed, 35}, {NeoWPF::UI::SizeMode::Star, 3}},
            {{NeoWPF::UI::SizeMode::Auto, 0}, {NeoWPF::UI::SizeMode::Fixed, 25}, {NeoWPF::UI::SizeMode::Star, 1}},
            {{0, 0, 1, 2, 30, 12}, {1, 1, 1, 1, 8, 18}, {2, 0, 1, 1, 13, 9}, {2, 2, 1, 1, 7, 11}},
            220, 140, 4, 6
        },
        {
            {{NeoWPF::UI::SizeMode::Fixed, 12}, {NeoWPF::UI::SizeMode::Star, 1}, {NeoWPF::UI::SizeMode::Fixed, 16}, {NeoWPF::UI::SizeMode::Star, 2}},
            {{NeoWPF::UI::SizeMode::Fixed, 18}, {NeoWPF::UI::SizeMode::Star, 2}, {NeoWPF::UI::SizeMode::Fixed, 22}, {NeoWPF::UI::SizeMode::Star, 1}},
            {{0, 0, 1, 1, 12, 12}, {0, 1, 1, 2, 9, 7}, {1, 3, 2, 1, 15, 8}, {3, 0, 1, 1, 6, 6}, {2, 2, 1, 1, 11, 10}},
            260, 180, 3, 4
        }
    };

    for (const auto& testCase : cases) {
        NeoWPF::UI::Grid grid;
        for (const auto& rowDef : testCase.rows) {
            grid.AddRow(rowDef.mode, rowDef.value);
        }
        for (const auto& colDef : testCase.columns) {
            grid.AddColumn(colDef.mode, colDef.value);
        }
        grid.SetSpacing(NeoWPF::UI::Spacing(testCase.spacingX, testCase.spacingY));

        std::vector<std::shared_ptr<FixedGridChild>> children;
        for (const auto& placement : testCase.placements) {
            auto child = std::make_shared<FixedGridChild>(placement.width, placement.height);
            grid.AddChild(child, placement.row, placement.column, placement.rowSpan, placement.columnSpan);
            children.push_back(child);
        }

        EXPECT_NO_THROW(grid.Measure(testCase.width, testCase.height));
        EXPECT_NO_THROW(grid.Arrange(testCase.width, testCase.height));

        for (const auto& child : children) {
            ExpectChildStaysInsideGrid(child, testCase.width, testCase.height);
        }
    }
}

TEST(GridTests, KeepsDenseSpanningLayoutsStableAcrossManyCombinations)
{
    const int widths[] = { 25, 12, 18, 9, 30 };
    const int heights[] = { 15, 10, 20, 8, 14 };

    for (int rowCount = 2; rowCount <= 5; ++rowCount) {
        for (int colCount = 2; colCount <= 5; ++colCount) {
            NeoWPF::UI::Grid grid;
            for (int r = 0; r < rowCount; ++r) {
                grid.AddRow(NeoWPF::UI::SizeMode::Fixed, 20 + r * 5);
            }
            for (int c = 0; c < colCount; ++c) {
                grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, 25 + c * 7);
            }
            grid.SetSpacing(NeoWPF::UI::Spacing(2, 3));

            std::vector<std::shared_ptr<FixedGridChild>> children;
            int childIndex = 0;
            for (int r = 0; r < rowCount; ++r) {
                for (int c = 0; c < colCount; ++c) {
                    auto child = std::make_shared<FixedGridChild>(widths[childIndex % 5], heights[childIndex % 5]);
                    const int rowSpan = ((r + c) % 2 == 0) ? 1 : 2;
                    const int colSpan = ((r + c) % 3 == 0) ? 1 : 2;
                    if (r + rowSpan > rowCount || c + colSpan > colCount) {
                        continue;
                    }
                    grid.AddChild(child, r, c, rowSpan, colSpan);
                    children.push_back(child);
                    ++childIndex;
                }
            }

            const int finalWidth = 200 + colCount * 10;
            const int finalHeight = 150 + rowCount * 12;
            EXPECT_NO_THROW(grid.Measure(finalWidth, finalHeight));
            EXPECT_NO_THROW(grid.Arrange(finalWidth, finalHeight));

            for (const auto& child : children) {
                ExpectChildStaysInsideGrid(child, finalWidth, finalHeight);
            }
        }
    }
}

TEST(GridTests, HandlesExtremeDefinitionsWithoutNegativeSizes)
{
    NeoWPF::UI::Grid grid;
    const int maximum = (std::numeric_limits<int>::max)();
    grid.AddRow(NeoWPF::UI::SizeMode::Fixed, -maximum);
    grid.AddRow(NeoWPF::UI::SizeMode::Star, maximum);
    grid.AddRow(NeoWPF::UI::SizeMode::Star, maximum);
    grid.AddColumn(NeoWPF::UI::SizeMode::Fixed, -maximum);
    grid.AddColumn(NeoWPF::UI::SizeMode::Star, maximum);
    grid.AddColumn(NeoWPF::UI::SizeMode::Star, maximum);

    auto child = std::make_shared<FixedGridChild>(1, 1);
    grid.AddChild(child, 1, 1);

    EXPECT_NO_THROW(grid.Measure(maximum, maximum));
    EXPECT_NO_THROW(grid.Arrange(maximum, maximum));
    EXPECT_GE(grid.GetDesiredWidth(), 0);
    EXPECT_GE(grid.GetDesiredHeight(), 0);
    EXPECT_GE(child->GetFinalWidth(), 0);
    EXPECT_GE(child->GetFinalHeight(), 0);
}

}