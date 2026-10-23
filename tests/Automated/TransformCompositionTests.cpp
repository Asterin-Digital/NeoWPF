#include <gtest/gtest.h>

#if defined(_WIN32)
#include <Windows.h>
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
    const auto& quad = commands.commands[0].texturedQuad;
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

}
