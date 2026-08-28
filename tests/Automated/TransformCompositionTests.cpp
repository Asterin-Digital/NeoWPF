#include <gtest/gtest.h>

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>

#include <memory>

namespace {

class TestElement final : public NeoWPF::UI::FrameworkElement {
public:
    void Measure(int, int) override
    {
        SetDesiredSize(100, 40);
    }
};

TEST(TransformCompositionTests, TranslationTransformsLocalPoint)
{
    TestElement element;
    element.SetLayoutOffset(10, 20);
    element.SetRenderTransform(std::make_shared<NeoWPF::UI::TranslateTransform>(5.0f, 7.0f));

    float x = 0.0f;
    float y = 0.0f;
    element.GetWorldTransformMatrix().TransformPoint(0.0f, 0.0f, x, y);

    EXPECT_FLOAT_EQ(x, 15.0f);
    EXPECT_FLOAT_EQ(y, 27.0f);
}

TEST(TransformCompositionTests, ChildWorldTransformIncludesParentTransform)
{
    auto parent = std::make_shared<TestElement>();
    auto child = std::make_shared<TestElement>();
    parent->SetLayoutOffset(10, 20);
    child->SetLayoutOffset(3, 4);
    parent->AddChild(child);

    float x = 0.0f;
    float y = 0.0f;
    child->GetWorldTransformMatrix().TransformPoint(0.0f, 0.0f, x, y);

    EXPECT_FLOAT_EQ(x, 13.0f);
    EXPECT_FLOAT_EQ(y, 24.0f);
}

TEST(TransformCompositionTests, CenterOriginScaleKeepsCenterFixed)
{
    TestElement element;
    element.Arrange(100, 40);
    element.SetRenderTransformOrigin({0.5f, 0.5f});
    element.SetRenderTransform(std::make_shared<NeoWPF::UI::ScaleTransform>(2.0f, 2.0f));

    float x = 0.0f;
    float y = 0.0f;
    element.GetWorldTransformMatrix().TransformPoint(50.0f, 20.0f, x, y);

    EXPECT_FLOAT_EQ(x, 50.0f);
    EXPECT_FLOAT_EQ(y, 20.0f);
}

}