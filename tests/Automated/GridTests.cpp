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

    void Measure(int, int) override
    {
        SetDesiredSize(width, height);
    }

private:
    int width;
    int height;
};

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