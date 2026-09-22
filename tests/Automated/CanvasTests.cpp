#include <gtest/gtest.h>

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Canvas/Canvas.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <cmath>
#include <limits>
#include <memory>
#include <typeindex>

namespace {

class FixedCanvasChild : public NeoWPF::UI::FrameworkElement {
public:
    FixedCanvasChild(double width, double height)
        : width(width), height(height) {}

    void Measure(double, double) override
    {
        SetDesiredSize(width, height);
    }

private:
    double width;
    double height;
};

class InteractiveCanvasChild final : public FixedCanvasChild {
public:
    using FixedCanvasChild::FixedCanvasChild;

    bool IsInteractable() const noexcept override { return true; }
};

class VisualOnlyChild final : public NeoWPF::UI::Visual {
};

TEST(CanvasTests, CanvasAttachedPropertiesHaveExpectedMetadata)
{
    const auto& left = NeoWPF::UI::Canvas::LeftProperty();
    const auto& top = NeoWPF::UI::Canvas::TopProperty();
    const auto& right = NeoWPF::UI::Canvas::RightProperty();
    const auto& bottom = NeoWPF::UI::Canvas::BottomProperty();

    EXPECT_EQ(left.GetName(), "Left");
    EXPECT_EQ(top.GetName(), "Top");
    EXPECT_EQ(right.GetName(), "Right");
    EXPECT_EQ(bottom.GetName(), "Bottom");
    EXPECT_EQ(left.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Canvas)));
    EXPECT_EQ(left.GetAttachedTargetType(), std::type_index(typeid(NeoWPF::UI::Visual)));
    EXPECT_EQ(left.GetPropertyType(), std::type_index(typeid(double)));
    EXPECT_TRUE(left.IsAttached());
    EXPECT_TRUE(std::isnan(NeoWPF::UI::Canvas::GetLeft(nullptr)));
}

TEST(CanvasTests, ArrangesChildrenAtLeftAndTop)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    canvas.AddChild(child);

    NeoWPF::UI::Canvas::SetLeft(child.get(), 18.5);
    NeoWPF::UI::Canvas::SetTop(child.get(), 27.25);
    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);

    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetX(), 18.5);
    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetY(), 27.25);
    EXPECT_DOUBLE_EQ(child->GetFinalWidth(), 40.0);
    EXPECT_DOUBLE_EQ(child->GetFinalHeight(), 30.0);
}

TEST(CanvasTests, UsesRightAndBottomWhenLeadingCoordinatesAreUnset)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    canvas.AddChild(child);

    NeoWPF::UI::Canvas::SetRight(child.get(), 12.0);
    NeoWPF::UI::Canvas::SetBottom(child.get(), 9.0);
    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);

    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetX(), 148.0);
    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetY(), 121.0);
}

TEST(CanvasTests, LeadingCoordinatesTakePrecedence)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    canvas.AddChild(child);

    NeoWPF::UI::Canvas::SetLeft(child.get(), 6.0);
    NeoWPF::UI::Canvas::SetRight(child.get(), 100.0);
    NeoWPF::UI::Canvas::SetTop(child.get(), 8.0);
    NeoWPF::UI::Canvas::SetBottom(child.get(), 100.0);
    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);

    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetX(), 6.0);
    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetY(), 8.0);
}

TEST(CanvasTests, ClearingCoordinateRestoresUnsetBehavior)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    canvas.AddChild(child);

    NeoWPF::UI::Canvas::SetLeft(child.get(), 25.0);
    NeoWPF::UI::Canvas::SetLeft(child.get(), std::numeric_limits<double>::quiet_NaN());
    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);

    EXPECT_TRUE(std::isnan(NeoWPF::UI::Canvas::GetLeft(child.get())));
    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetX(), 0.0);
}

TEST(CanvasTests, PreservesNegativeAndVeryLargeCoordinates)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    canvas.AddChild(child);

    NeoWPF::UI::Canvas::SetLeft(child.get(), -1000000000.25);
    NeoWPF::UI::Canvas::SetTop(child.get(), 1000000000.5);
    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);

    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetX(), -1000000000.25);
    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetY(), 1000000000.5);
}

TEST(CanvasTests, AppliesMarginsAfterCoordinateResolution)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    child->SetMargin(3.0, 4.0, 5.0, 6.0);
    canvas.AddChild(child);

    NeoWPF::UI::Canvas::SetLeft(child.get(), 18.0);
    NeoWPF::UI::Canvas::SetTop(child.get(), 27.0);
    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);

    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetX(), 21.0);
    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetY(), 31.0);
    EXPECT_DOUBLE_EQ(child->GetFinalWidth(), 40.0);
    EXPECT_DOUBLE_EQ(child->GetFinalHeight(), 30.0);
}

TEST(CanvasTests, UsesExplicitChildSizeInsteadOfDesiredSize)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    child->SetWidth(75.0);
    child->SetHeight(55.0);
    canvas.AddChild(child);

    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);

    EXPECT_DOUBLE_EQ(child->GetFinalWidth(), 75.0);
    EXPECT_DOUBLE_EQ(child->GetFinalHeight(), 55.0);
}

TEST(CanvasTests, CoordinateChangesInvalidateArrange)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    canvas.AddChild(child);
    child->ClearLayoutDirty();

    NeoWPF::UI::Canvas::SetLeft(child.get(), 33.0);

    EXPECT_TRUE(child->IsLayoutDirty());
}

TEST(CanvasTests, RepeatedArrangeDoesNotAccumulateOffsets)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    canvas.AddChild(child);
    NeoWPF::UI::Canvas::SetLeft(child.get(), 12.0);
    NeoWPF::UI::Canvas::SetTop(child.get(), 14.0);

    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);
    canvas.Arrange(120.0, 100.0);

    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetX(), 12.0);
    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetY(), 14.0);
}

TEST(CanvasTests, SkipsInvisibleChildrenDuringMeasureAndArrange)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<FixedCanvasChild>(40.0, 30.0);
    canvas.AddChild(child);
    child->SetVisibility(false);
    NeoWPF::UI::Canvas::SetLeft(child.get(), 20.0);
    NeoWPF::UI::Canvas::SetTop(child.get(), 25.0);

    EXPECT_NO_THROW(canvas.Measure(200.0, 160.0));
    EXPECT_NO_THROW(canvas.Arrange(200.0, 160.0));
    EXPECT_DOUBLE_EQ(child->GetFinalWidth(), 0.0);
    EXPECT_DOUBLE_EQ(child->GetFinalHeight(), 0.0);
}

TEST(CanvasTests, IgnoresVisualChildrenThatCannotBeArranged)
{
    NeoWPF::UI::Canvas canvas;
    auto visual = std::make_shared<VisualOnlyChild>();
    canvas.AddVisualChild(visual);

    EXPECT_NO_THROW(canvas.Measure(200.0, 160.0));
    EXPECT_NO_THROW(canvas.Arrange(200.0, 160.0));
    EXPECT_EQ(visual->GetParent(), &canvas);
}

TEST(CanvasTests, OverlappingChildrenUseZOrderForHitTesting)
{
    NeoWPF::UI::Canvas canvas;
    auto lower = std::make_shared<InteractiveCanvasChild>(40.0, 30.0);
    auto upper = std::make_shared<InteractiveCanvasChild>(40.0, 30.0);
    canvas.AddChild(lower);
    canvas.AddChild(upper);
    lower->SetZIndex(1);
    upper->SetZIndex(2);

    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);

    auto& input = NeoWPF::Input::InputManager::GetInstance();
    input.SetRootElement(&canvas);
    EXPECT_EQ(input.HitTest(10, 10), upper.get());
    input.SetRootElement(nullptr);
}

TEST(CanvasTests, HitTestingUsesUpdatedCoordinates)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<InteractiveCanvasChild>(40.0, 30.0);
    canvas.AddChild(child);
    NeoWPF::UI::Canvas::SetLeft(child.get(), 50.0);
    NeoWPF::UI::Canvas::SetTop(child.get(), 60.0);

    canvas.Measure(200.0, 160.0);
    canvas.Arrange(200.0, 160.0);

    auto& input = NeoWPF::Input::InputManager::GetInstance();
    input.SetRootElement(&canvas);
    EXPECT_EQ(input.HitTest(55, 65), child.get());
    EXPECT_EQ(input.HitTest(5, 5), nullptr);
    input.SetRootElement(nullptr);
}

}