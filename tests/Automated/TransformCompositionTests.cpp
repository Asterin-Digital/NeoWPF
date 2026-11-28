#include <gtest/gtest.h>

#if defined(_WIN32)
#include <windows.h>
#ifdef CreateWindow
#undef CreateWindow
#endif
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Rendering/Renderer.h>
#endif
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>

#include <memory>

namespace {

class TestElement final : public NeoWPF::UI::FrameworkElement {
public:
    void Measure(double, double) override
    {
        SetDesiredSize(100, 40);
    }
};

TEST(TransformCompositionTests, TranslationTransformsLocalPoint)
{
    TestElement element;
    element.SetLayoutOffset(10, 20);
    element.SetRenderTransform(std::make_shared<NeoWPF::UI::TranslateTransform>(5.0f, 7.0f));

    double x = 0.0;
    double y = 0.0;
    element.GetWorldTransformMatrix().TransformPoint(0.0, 0.0, x, y);

    EXPECT_DOUBLE_EQ(x, 15.0);
    EXPECT_DOUBLE_EQ(y, 27.0);
}

TEST(TransformCompositionTests, RenderTransformChangesInvalidateTheCachedWorldMatrix)
{
    TestElement element;
    auto translation = std::make_shared<NeoWPF::UI::TranslateTransform>();
    element.SetRenderTransform(translation);

    double beforeX = 0.0;
    double beforeY = 0.0;
    element.GetWorldTransformMatrix().TransformPoint(0.0, 0.0, beforeX, beforeY);

    translation->SetX(13.0f);
    double afterX = 0.0;
    double afterY = 0.0;
    element.GetWorldTransformMatrix().TransformPoint(0.0, 0.0, afterX, afterY);

    EXPECT_DOUBLE_EQ(beforeX, 0.0);
    EXPECT_DOUBLE_EQ(afterX, 13.0);
    EXPECT_DOUBLE_EQ(afterY, 0.0);
}

TEST(TransformCompositionTests, TransformDispatchSupportsMultipleSubscribers)
{
    auto transform = std::make_shared<NeoWPF::UI::TranslateTransform>();
    std::size_t firstCount = 0;
    std::size_t secondCount = 0;
    NeoWPF::UI::Core::TransformSubscription first;
    NeoWPF::UI::Core::TransformSubscription second;
    first.Subscribe(transform, [&] { ++firstCount; });
    second.Subscribe(transform, [&] { ++secondCount; });

    transform->SetY(9.0f);

    EXPECT_EQ(firstCount, 1u);
    EXPECT_EQ(secondCount, 1u);
}

TEST(TransformCompositionTests, ChildWorldTransformIncludesParentTransform)
{
    auto parent = std::make_shared<TestElement>();
    auto child = std::make_shared<TestElement>();
    parent->SetLayoutOffset(10, 20);
    child->SetLayoutOffset(3, 4);
    parent->AddChild(child);

    double x = 0.0;
    double y = 0.0;
    child->GetWorldTransformMatrix().TransformPoint(0.0, 0.0, x, y);

    EXPECT_DOUBLE_EQ(x, 13.0);
    EXPECT_DOUBLE_EQ(y, 24.0);
}

TEST(TransformCompositionTests, CenterOriginScaleKeepsCenterFixed)
{
    TestElement element;
    element.Arrange(100, 40);
    element.SetRenderTransformOrigin({0.5f, 0.5f});
    element.SetRenderTransform(std::make_shared<NeoWPF::UI::ScaleTransform>(2.0f, 2.0f));

    double x = 0.0;
    double y = 0.0;
    element.GetWorldTransformMatrix().TransformPoint(50.0, 20.0, x, y);

    EXPECT_DOUBLE_EQ(x, 50.0);
    EXPECT_DOUBLE_EQ(y, 20.0);
}

#if defined(_WIN32)
TEST(TransformCompositionTests, TextBlockUsesUnrotatedAbsoluteOriginWhenParentRotates)
{
    NeoWPF::Core::Application::Initialize();
    auto testWindow = NeoWPF::Core::Application::Current().CreateWindow(L"NeoWPF transform composition test", 320, 240);
    ASSERT_NE(testWindow.Handle(), nullptr);
    ::ShowWindow(static_cast<HWND>(testWindow.Handle()), SW_HIDE);
    NeoWPF::Rendering::Renderer_Initialize(NeoWPF::Rendering::RenderSurface{testWindow.Handle()});
    ASSERT_TRUE(NeoWPF::Rendering::Renderer_IsInitialized());

    auto parent = std::make_shared<TestElement>();
    parent->Arrange(200, 120);
    parent->SetRenderTransform(std::make_shared<NeoWPF::UI::RotateTransform>(1.57079632679f));

    auto text = std::make_shared<NeoWPF::UI::TextBlock>(L"Hello");
    text->Arrange(80, 30);
    parent->AddChild(text);
    text->Arrange(80, 30);

    NeoWPF::Rendering::CommandList commands;
    text->BuildRenderCommands(commands);

    ASSERT_FALSE(commands.commands.empty());
    const auto& quad = commands.commands[0].GetTexturedQuad();
    const auto origin = parent->GetRenderTransformOrigin();
    const float expectedPivotX = static_cast<float>(parent->GetFinalWidth()) * origin.X;
    const float expectedPivotY = static_cast<float>(parent->GetFinalHeight()) * origin.Y;

    EXPECT_FLOAT_EQ(quad.rotationRadians, parent->GetWorldRotationRadians());
    EXPECT_FLOAT_EQ(quad.pivotX, expectedPivotX);
    EXPECT_FLOAT_EQ(quad.pivotY, expectedPivotY);

    NeoWPF::Rendering::Renderer_Shutdown();
    NeoWPF::Core::Application::Shutdown();
}
#endif

TEST(TransformCompositionTests, TryGetRotationReportsSupportedTransformValues)
{
    NeoWPF::UI::ScaleTransform scale(2.0f, 2.0f);
    EXPECT_FALSE(scale.TryGetRotation().has_value());

    NeoWPF::UI::RotateTransform rotation(3.5f);
    const auto rotationValue = rotation.TryGetRotation();
    ASSERT_TRUE(rotationValue.has_value());
    EXPECT_FLOAT_EQ(*rotationValue, 3.5f);
}

TEST(TransformCompositionTests, TryGetRotationSumsRotationGroup)
{
    NeoWPF::UI::TransformGroup rotations;
    rotations.AddChild(std::make_shared<NeoWPF::UI::RotateTransform>(1.25f));
    rotations.AddChild(std::make_shared<NeoWPF::UI::RotateTransform>(2.25f));

    const auto rotationValue = rotations.TryGetRotation();
    ASSERT_TRUE(rotationValue.has_value());
    EXPECT_FLOAT_EQ(*rotationValue, 3.5f);
}

TEST(TransformCompositionTests, TryGetRotationRejectsMixedTransformGroup)
{
    NeoWPF::UI::TransformGroup mixed;
    mixed.AddChild(std::make_shared<NeoWPF::UI::RotateTransform>(1.25f));
    mixed.AddChild(std::make_shared<NeoWPF::UI::ScaleTransform>(2.0f, 2.0f));

    EXPECT_FALSE(mixed.TryGetRotation().has_value());
}

TEST(TransformCompositionTests, Matrix4ComputesAnInverseForAffine2DTransforms)
{
    const auto matrix = NeoWPF::UI::Matrix4::Translation(12.0f, -7.0f)
        * NeoWPF::UI::Matrix4::Rotation(0.4f)
        * NeoWPF::UI::Matrix4::Scale(2.0f, 3.0f);
    const auto inverse = matrix.TryInverse2D();
    ASSERT_TRUE(inverse.has_value());

    double x = 0.0;
    double y = 0.0;
    matrix.TransformPoint(5.0, -9.0, x, y);
    inverse->TransformPoint(x, y, x, y);

    EXPECT_NEAR(x, 5.0, 1e-5);
    EXPECT_NEAR(y, -9.0, 1e-5);
}

TEST(TransformCompositionTests, Matrix4RejectsSingularAndNon2DTransforms)
{
    EXPECT_FALSE(NeoWPF::UI::Matrix4::Scale(0.0f, 1.0f).TryInverse2D().has_value());
    auto perspective = NeoWPF::UI::Matrix4::Identity();
    perspective.m[0][3] = 0.25f;
    EXPECT_FALSE(perspective.TryInverse2D().has_value());
}

TEST(TransformCompositionTests, TransformGroupRelaysChildChanges)
{
    NeoWPF::UI::TransformGroup group;
    auto translation = std::make_shared<NeoWPF::UI::TranslateTransform>();
    group.AddChild(translation);

    std::size_t changeCount = 0;
    const auto handlerId = group.AddChangedHandler([&] {
        ++changeCount;
    });

    translation->SetX(3.0f);
    group.ClearChildren();
    translation->SetY(5.0f);

    group.RemoveChangedHandler(handlerId);
    EXPECT_EQ(changeCount, 2u);
}

}
