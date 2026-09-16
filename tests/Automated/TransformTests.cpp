#include <gtest/gtest.h>

#if defined(_WIN32)
#include <Windows.h>
#ifdef CreateWindow
#undef CreateWindow
#endif
#endif

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <cmath>
#include <utility>

namespace {

#if defined(_WIN32)

class TextBlockRenderCommandTest : public ::testing::Test {
protected:
    static void SetUpTestSuite()
    {
        NeoWPF::Core::Application::Initialize();
        testWindow = NeoWPF::Core::Application::Current().CreateWindow(L"NeoWPF transform test", 320, 240);
        ASSERT_NE(testWindow.Handle(), nullptr);

        ::ShowWindow(static_cast<HWND>(testWindow.Handle()), SW_HIDE);
        NeoWPF::Rendering::Renderer_Initialize(testWindow.Handle());
        ASSERT_TRUE(NeoWPF::Rendering::Renderer_IsInitialized());
    }

    static void TearDownTestSuite()
    {
        NeoWPF::Rendering::Renderer_Shutdown();
        NeoWPF::Core::Application::Shutdown();
    }

    static NeoWPF::Platform::Window testWindow;
};

NeoWPF::Platform::Window TextBlockRenderCommandTest::testWindow;

using TexturedQuadData = decltype(std::declval<NeoWPF::Rendering::CommandList::Command>().texturedQuad);
using RoundedRectData = decltype(std::declval<NeoWPF::Rendering::CommandList::Command>().roundedRect);

const TexturedQuadData* FindTextQuad(
    const NeoWPF::Rendering::CommandList& commands)
{
    for (const auto& command : commands.commands) {
        if (command.kind == NeoWPF::Rendering::CommandList::CommandKind::TexturedQuad) {
            return &command.texturedQuad;
        }
    }
    return nullptr;
}

const RoundedRectData* FindRoundedRect(
    const NeoWPF::Rendering::CommandList& commands)
{
    for (const auto& command : commands.commands) {
        if (command.kind == NeoWPF::Rendering::CommandList::CommandKind::RoundedRect) {
            return &command.roundedRect;
        }
    }
    return nullptr;
}

float SmoothStep(float progress)
{
    return progress * progress * (3.0f - 2.0f * progress);
}

void RotatePointAroundPivot(
    float x,
    float y,
    float pivotX,
    float pivotY,
    float angle,
    float& outX,
    float& outY)
{
    const float cosAngle = std::cos(angle);
    const float sinAngle = std::sin(angle);
    const float offsetX = x - pivotX;
    const float offsetY = y - pivotY;
    outX = pivotX + offsetX * cosAngle - offsetY * sinAngle;
    outY = pivotY + offsetX * sinAngle + offsetY * cosAngle;
}

TEST_F(TextBlockRenderCommandTest, ScaleTransformsBitmapQuadAroundArrangedCenter)
{
    NeoWPF::UI::TextBlock textBlock(L"Transform me");
    textBlock.Arrange(100, 40);

    NeoWPF::Rendering::CommandList unscaledCommands;
    textBlock.BuildRenderCommands(unscaledCommands);
    const auto* unscaled = FindTextQuad(unscaledCommands);
    ASSERT_NE(unscaled, nullptr);
    ASSERT_GT(unscaled->textureId, 0u);

    textBlock.SetRenderTransformOrigin({0.5f, 0.5f});
    textBlock.SetRenderTransform(std::make_shared<NeoWPF::UI::ScaleTransform>(2.0f, 2.0f));

    NeoWPF::Rendering::CommandList scaledCommands;
    textBlock.BuildRenderCommands(scaledCommands);
    const auto* scaled = FindTextQuad(scaledCommands);
    ASSERT_NE(scaled, nullptr);
    ASSERT_EQ(scaled->textureId, unscaled->textureId);

    EXPECT_FLOAT_EQ(scaled->w, unscaled->w * 2.0f);
    EXPECT_FLOAT_EQ(scaled->h, unscaled->h * 2.0f);

    const float unscaledCenterX = unscaled->x + unscaled->w * 0.5f;
    const float unscaledCenterY = unscaled->y + unscaled->h * 0.5f;
    const float scaledCenterX = scaled->x + scaled->w * 0.5f;
    const float scaledCenterY = scaled->y + scaled->h * 0.5f;
    EXPECT_FLOAT_EQ(scaledCenterX, 50.0f + (unscaledCenterX - 50.0f) * 2.0f);
    EXPECT_FLOAT_EQ(scaledCenterY, 20.0f + (unscaledCenterY - 20.0f) * 2.0f);
}

TEST_F(TextBlockRenderCommandTest, TopLeftOriginKeepsBitmapQuadAnchorFixed)
{
    NeoWPF::UI::TextBlock textBlock(L"Transform me");
    textBlock.Arrange(100, 40);

    textBlock.SetRenderTransformOrigin({0.0f, 0.0f});
    textBlock.SetRenderTransform(std::make_shared<NeoWPF::UI::ScaleTransform>(2.0f, 2.0f));

    NeoWPF::Rendering::CommandList commands;
    textBlock.BuildRenderCommands(commands);
    const auto* quad = FindTextQuad(commands);
    ASSERT_NE(quad, nullptr);
    ASSERT_GT(quad->textureId, 0u);

    EXPECT_FLOAT_EQ(quad->x, 0.0f);
    EXPECT_FLOAT_EQ(quad->y, 0.0f);
}

TEST_F(TextBlockRenderCommandTest, ButtonBackgroundAndBitmapTextShareScaleTransform)
{
    NeoWPF::UI::Button button(L"Transform me");
    button.Measure(100, 40);
    button.Arrange(100, 40);

    NeoWPF::Rendering::CommandList unscaledCommands;
    button.BuildRenderCommands(unscaledCommands);
    const auto* unscaledBackground = FindRoundedRect(unscaledCommands);
    const auto* unscaledText = FindTextQuad(unscaledCommands);
    ASSERT_NE(unscaledBackground, nullptr);
    ASSERT_NE(unscaledText, nullptr);
    ASSERT_GT(unscaledText->textureId, 0u);

    button.SetRenderTransformOrigin({0.5f, 0.5f});
    button.SetRenderTransform(std::make_shared<NeoWPF::UI::ScaleTransform>(2.0f, 2.0f));

    NeoWPF::Rendering::CommandList scaledCommands;
    button.BuildRenderCommands(scaledCommands);
    const auto* scaledBackground = FindRoundedRect(scaledCommands);
    const auto* scaledText = FindTextQuad(scaledCommands);
    ASSERT_NE(scaledBackground, nullptr);
    ASSERT_NE(scaledText, nullptr);
    ASSERT_EQ(scaledText->textureId, unscaledText->textureId);

    const float unscaledBackgroundCenterX = unscaledBackground->x + unscaledBackground->w * 0.5f;
    const float unscaledBackgroundCenterY = unscaledBackground->y + unscaledBackground->h * 0.5f;
    const float scaledBackgroundCenterX = scaledBackground->x + scaledBackground->w * 0.5f;
    const float scaledBackgroundCenterY = scaledBackground->y + scaledBackground->h * 0.5f;
    EXPECT_FLOAT_EQ(scaledBackgroundCenterX, 50.0f + (unscaledBackgroundCenterX - 50.0f) * 2.0f);
    EXPECT_FLOAT_EQ(scaledBackgroundCenterY, 20.0f + (unscaledBackgroundCenterY - 20.0f) * 2.0f);

    const float unscaledTextCenterX = unscaledText->x + unscaledText->w * 0.5f;
    const float unscaledTextCenterY = unscaledText->y + unscaledText->h * 0.5f;
    const float scaledTextCenterX = scaledText->x + scaledText->w * 0.5f;
    const float scaledTextCenterY = scaledText->y + scaledText->h * 0.5f;
    EXPECT_FLOAT_EQ(scaledText->w, unscaledText->w * 2.0f);
    EXPECT_FLOAT_EQ(scaledText->h, unscaledText->h * 2.0f);
    EXPECT_FLOAT_EQ(scaledTextCenterX, 50.0f + (unscaledTextCenterX - 50.0f) * 2.0f);
    EXPECT_FLOAT_EQ(scaledTextCenterY, 20.0f + (unscaledTextCenterY - 20.0f) * 2.0f);
}

TEST_F(TextBlockRenderCommandTest, ButtonRotationKeepsTextCenteredThroughEasedLoop)
{
    NeoWPF::UI::Button button(L"Spin me");
    button.Measure(160, 60);
    button.Arrange(160, 60);

    auto rotation = std::make_shared<NeoWPF::UI::RotateTransform>();
    button.SetRenderTransformOrigin({0.5f, 0.5f});
    button.SetRenderTransform(rotation);

    constexpr float pi = 3.14159265358979323846f;
    constexpr float centerX = 80.0f;
    constexpr float centerY = 30.0f;
    for (int frame = 0; frame <= 120; ++frame) {
        const float progress = static_cast<float>(frame) / 120.0f;
        const float angle = SmoothStep(progress) * 2.0f * pi;
        rotation->SetAngle(angle);

        NeoWPF::Rendering::CommandList commands;
        button.BuildRenderCommands(commands);
        const auto* background = FindRoundedRect(commands);
        const auto* text = FindTextQuad(commands);
        ASSERT_NE(background, nullptr);
        ASSERT_NE(text, nullptr);
        ASSERT_GT(text->textureId, 0u);

        EXPECT_NEAR(background->x + background->w * 0.5f, centerX, 0.51f);
        EXPECT_NEAR(background->y + background->h * 0.5f, centerY, 0.51f);
        EXPECT_NEAR(text->pivotX, centerX, 0.01f);
        EXPECT_NEAR(text->pivotY, centerY, 0.01f);
        EXPECT_NEAR(text->rotationRadians, angle, 0.0001f);
    }
}

TEST_F(TextBlockRenderCommandTest, TextBlockRotationDoesNotDriftThroughEasedLoop)
{
    NeoWPF::UI::TextBlock textBlock(L"Spin me");
    textBlock.Measure(160, 60);
    textBlock.Arrange(160, 60);

    auto rotation = std::make_shared<NeoWPF::UI::RotateTransform>();
    textBlock.SetRenderTransformOrigin({0.5f, 0.5f});
    textBlock.SetRenderTransform(rotation);

    constexpr float pi = 3.14159265358979323846f;
    constexpr float centerX = 80.0f;
    constexpr float centerY = 30.0f;
    for (int frame = 0; frame <= 120; ++frame) {
        const float progress = static_cast<float>(frame) / 120.0f;
        const float angle = SmoothStep(progress) * 2.0f * pi;
        rotation->SetAngle(angle);

        NeoWPF::Rendering::CommandList commands;
        textBlock.BuildRenderCommands(commands);
        const auto* text = FindTextQuad(commands);
        ASSERT_NE(text, nullptr);
        ASSERT_GT(text->textureId, 0u);

        float transformedCenterX = 0.0f;
        float transformedCenterY = 0.0f;
        RotatePointAroundPivot(
            text->x + text->w * 0.5f,
            text->y + text->h * 0.5f,
            text->pivotX,
            text->pivotY,
            text->rotationRadians,
            transformedCenterX,
            transformedCenterY);
        EXPECT_NEAR(transformedCenterX, centerX, 0.01f);
        EXPECT_NEAR(transformedCenterY, centerY, 0.01f);
        EXPECT_NEAR(text->pivotX, centerX, 0.01f);
        EXPECT_NEAR(text->pivotY, centerY, 0.01f);
    }
}

#endif

} // namespace