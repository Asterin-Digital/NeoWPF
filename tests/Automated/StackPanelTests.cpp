#include <gtest/gtest.h>

#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>

#include <memory>

namespace {

class FixedElement final : public NeoWPF::UI::FrameworkElement {
public:
    FixedElement(double width, double height)
        : m_width(width), m_height(height)
    {
        SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Left);
        SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Top);
    }

    void Measure(double, double) override
    {
        SetDesiredSize(m_width, m_height);
    }

private:
    double m_width;
    double m_height;
};

TEST(StackPanelTests, DefaultsToVerticalOrientation)
{
    NeoWPF::UI::StackPanel panel;

    EXPECT_EQ(panel.GetOrientation(), NeoWPF::UI::Orientation::Vertical);
}

TEST(StackPanelTests, MeasuresVerticalChildrenWithSpacing)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetSpacing(6.0);
    auto first = std::make_shared<FixedElement>(20.0, 10.0);
    auto second = std::make_shared<FixedElement>(35.0, 16.0);
    auto third = std::make_shared<FixedElement>(28.0, 12.0);
    panel.AddChild(first);
    panel.AddChild(second);
    panel.AddChild(third);

    panel.Measure(200.0, 200.0);
    panel.Arrange(200.0, 200.0);

    EXPECT_DOUBLE_EQ(panel.GetDesiredWidth(), 35.0);
    EXPECT_DOUBLE_EQ(panel.GetDesiredHeight(), 10.0 + 6.0 + 16.0 + 6.0 + 12.0);
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetY(), 0.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetY(), 16.0);
    EXPECT_DOUBLE_EQ(third->GetLayoutOffsetY(), 38.0);
}

TEST(StackPanelTests, MeasuresHorizontalChildrenWithSpacing)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    panel.SetSpacing(5.0);
    auto first = std::make_shared<FixedElement>(20.0, 10.0);
    auto second = std::make_shared<FixedElement>(35.0, 16.0);
    auto third = std::make_shared<FixedElement>(28.0, 12.0);
    panel.AddChild(first);
    panel.AddChild(second);
    panel.AddChild(third);

    panel.Measure(200.0, 200.0);
    panel.Arrange(200.0, 200.0);

    EXPECT_DOUBLE_EQ(panel.GetDesiredWidth(), 20.0 + 5.0 + 35.0 + 5.0 + 28.0);
    EXPECT_DOUBLE_EQ(panel.GetDesiredHeight(), 16.0);
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetX(), 0.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetX(), 25.0);
    EXPECT_DOUBLE_EQ(third->GetLayoutOffsetX(), 65.0);
}

TEST(StackPanelTests, IncludesMarginsInExtentAndOffsets)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetSpacing(4.0);
    auto first = std::make_shared<FixedElement>(20.0, 10.0);
    first->SetMargin(2.0, 3.0, 4.0, 5.0);
    auto second = std::make_shared<FixedElement>(30.0, 12.0);
    second->SetMargin(1.0, 2.0, 3.0, 4.0);
    panel.AddChild(first);
    panel.AddChild(second);

    panel.Measure(200.0, 200.0);
    panel.Arrange(200.0, 200.0);

    EXPECT_DOUBLE_EQ(panel.GetDesiredWidth(), 30.0 + 1.0 + 3.0);
    EXPECT_DOUBLE_EQ(panel.GetDesiredHeight(), (3.0 + 10.0 + 5.0) + 4.0 + (2.0 + 12.0 + 4.0));
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetX(), 2.0);
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetY(), 3.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetX(), 1.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetY(), 3.0 + 10.0 + 5.0 + 4.0 + 2.0);
}

TEST(StackPanelTests, HonorsCrossAxisAlignment)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetWidth(120.0);
    panel.SetHeight(100.0);
    auto centered = std::make_shared<FixedElement>(20.0, 10.0);
    centered->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);
    auto stretched = std::make_shared<FixedElement>(20.0, 12.0);
    stretched->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Stretch);
    panel.AddChild(centered);
    panel.AddChild(stretched);

    panel.Measure(120.0, 100.0);
    panel.Arrange(120.0, 100.0);

    EXPECT_DOUBLE_EQ(centered->GetLayoutOffsetX(), 50.0);
    EXPECT_DOUBLE_EQ(centered->GetFinalWidth(), 20.0);
    EXPECT_DOUBLE_EQ(stretched->GetLayoutOffsetX(), 0.0);
    EXPECT_DOUBLE_EQ(stretched->GetFinalWidth(), 120.0);
}

TEST(StackPanelTests, IgnoresInvisibleChildrenForSpacingAndPlacement)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetSpacing(8.0);
    auto first = std::make_shared<FixedElement>(20.0, 10.0);
    auto hidden = std::make_shared<FixedElement>(80.0, 80.0);
    hidden->SetVisibility(false);
    auto last = std::make_shared<FixedElement>(30.0, 12.0);
    panel.AddChild(first);
    panel.AddChild(hidden);
    panel.AddChild(last);

    panel.Measure(200.0, 200.0);
    panel.Arrange(200.0, 200.0);

    EXPECT_DOUBLE_EQ(panel.GetDesiredHeight(), 10.0 + 8.0 + 12.0);
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetY(), 0.0);
    EXPECT_DOUBLE_EQ(last->GetLayoutOffsetY(), 18.0);
}

TEST(StackPanelTests, SupportsMixedControlsAndNestedOrientation)
{
    auto root = std::make_shared<NeoWPF::UI::StackPanel>();
    root->SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    root->SetSpacing(10.0);

    auto button = std::make_shared<NeoWPF::UI::Button>(L"Play");
    button->SetWidth(72.0);
    button->SetHeight(30.0);
    auto nested = std::make_shared<NeoWPF::UI::StackPanel>();
    nested->SetSpacing(3.0);
    auto label = std::make_shared<NeoWPF::UI::TextBlock>(L"Mode");
    auto checkbox = std::make_shared<NeoWPF::UI::Checkbox>();
    checkbox->SetText(L"Sound");
    nested->AddChild(label);
    nested->AddChild(checkbox);
    root->AddChild(button);
    root->AddChild(nested);

    root->Measure(300.0, 200.0);
    root->Arrange(300.0, 200.0);

    ASSERT_EQ(root->GetVisualChildren().size(), 2u);
    EXPECT_EQ(root->GetVisualChildren()[0], button.get());
    EXPECT_EQ(root->GetVisualChildren()[1], nested.get());
    EXPECT_EQ(button->GetParent(), root.get());
    EXPECT_EQ(label->GetParent(), nested.get());
    EXPECT_EQ(checkbox->GetParent(), nested.get());
    EXPECT_GT(nested->GetFinalWidth(), 0.0);
    EXPECT_GT(nested->GetFinalHeight(), 0.0);
}

TEST(StackPanelTests, ReMeasuresAfterOrientationAndSpacingChanges)
{
    NeoWPF::UI::StackPanel panel;
    auto first = std::make_shared<FixedElement>(20.0, 10.0);
    auto second = std::make_shared<FixedElement>(30.0, 15.0);
    panel.AddChild(first);
    panel.AddChild(second);

    panel.Measure(200.0, 200.0);
    EXPECT_DOUBLE_EQ(panel.GetDesiredWidth(), 30.0);
    EXPECT_DOUBLE_EQ(panel.GetDesiredHeight(), 25.0);

    panel.SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    panel.SetSpacing(9.0);
    panel.Measure(200.0, 200.0);
    panel.Arrange(200.0, 200.0);

    EXPECT_DOUBLE_EQ(panel.GetDesiredWidth(), 20.0 + 9.0 + 30.0);
    EXPECT_DOUBLE_EQ(panel.GetDesiredHeight(), 15.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetX(), 29.0);
}

TEST(StackPanelTests, HandlesDeepAlternatingNestedStacks)
{
    auto root = std::make_shared<NeoWPF::UI::StackPanel>();
    auto current = root;
    constexpr int depth = 80;
    for (int index = 0; index < depth; ++index) {
        current->SetOrientation(index % 2 == 0
            ? NeoWPF::UI::Orientation::Vertical
            : NeoWPF::UI::Orientation::Horizontal);
        auto child = std::make_shared<NeoWPF::UI::StackPanel>();
        child->SetSpacing(static_cast<double>(index % 4));
        current->AddChild(child);
        current = child;
    }
    auto leaf = std::make_shared<FixedElement>(12.0, 9.0);
    current->AddChild(leaf);

    root->Measure(500.0, 500.0);
    root->Arrange(500.0, 500.0);

    EXPECT_DOUBLE_EQ(leaf->GetFinalWidth(), 12.0);
    EXPECT_DOUBLE_EQ(leaf->GetFinalHeight(), 9.0);
    EXPECT_EQ(root->GetVisualDescendants().size(), static_cast<std::size_t>(depth + 1));
}

TEST(StackPanelTests, PreservesLargeChildSet)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    panel.SetSpacing(1.0);

    constexpr int childCount = 128;
    for (int index = 0; index < childCount; ++index) {
        panel.AddChild(std::make_shared<FixedElement>(2.0, 3.0));
    }

    panel.Measure(1000.0, 50.0);
    panel.Arrange(1000.0, 50.0);

    EXPECT_EQ(panel.GetVisualChildren().size(), static_cast<std::size_t>(childCount));
    EXPECT_DOUBLE_EQ(panel.GetDesiredWidth(), childCount * 2.0 + (childCount - 1));
}

}
