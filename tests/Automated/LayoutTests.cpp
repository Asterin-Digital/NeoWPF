#include <gtest/gtest.h>

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>

#include <memory>

namespace {

class FixedElement final : public NeoWPF::UI::FrameworkElement {
public:
    FixedElement(int width, int height) : width(width), height(height)
    {
        SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Left);
        SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Top);
    }

    void Measure(int, int) override
    {
        SetDesiredSize(width, height);
    }

private:
    int width;
    int height;
};

TEST(LayoutTests, StackPanelMeasuresChildrenAndAddsSpacing)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetSpacing(5);

    auto first = std::make_shared<FixedElement>(20, 10);
    auto second = std::make_shared<FixedElement>(30, 15);
    panel.AddChild(first);
    panel.AddChild(second);

    panel.Measure(100, 100);
    EXPECT_EQ(panel.GetDesiredWidth(), 30);
    EXPECT_EQ(panel.GetDesiredHeight(), 30);
}

TEST(LayoutTests, StackPanelArrangesChildrenInOrder)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetSpacing(5);

    auto first = std::make_shared<FixedElement>(20, 10);
    auto second = std::make_shared<FixedElement>(30, 15);
    panel.AddChild(first);
    panel.AddChild(second);

    panel.Measure(100, 100);
    panel.Arrange(100, 100);

    EXPECT_EQ(first->GetLayoutOffsetX(), 0);
    EXPECT_EQ(first->GetLayoutOffsetY(), 0);
    EXPECT_EQ(second->GetLayoutOffsetX(), 0);
    EXPECT_EQ(second->GetLayoutOffsetY(), 15);
    EXPECT_EQ(first->GetFinalWidth(), 20);
    EXPECT_EQ(first->GetFinalHeight(), 10);
    EXPECT_EQ(second->GetFinalWidth(), 30);
    EXPECT_EQ(second->GetFinalHeight(), 15);
}

}