#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/GradientBrush.h>
#include <NeoWPF/UI/Core/Media/GradientStop.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Layout/Border/Border.h>

#include <memory>

namespace {

std::shared_ptr<NeoWPF::UI::LinearGradientBrush> CreateGradient(
    std::uint32_t firstColor,
    std::uint32_t lastColor)
{
    auto brush = std::make_shared<NeoWPF::UI::LinearGradientBrush>();
    brush->GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(firstColor), 0.0));
    brush->GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(lastColor), 1.0));
    return brush;
}

NeoWPF::Rendering::CommandList CreateWindowCommands(
    const std::shared_ptr<NeoWPF::UI::Brush>& backgroundBrush)
{
    NeoWPF::Core::WindowSession session;
    session.windowHost = std::make_shared<NeoWPF::UI::Window>();
    session.windowHost->SetBackgroundBrush(backgroundBrush);
    NeoWPF::Core::BuildWindowCommands(session, 100, 60, false);
    return session.buildCommandList;
}

TEST(WindowBackgroundPaintTests, SolidBackgroundStillSetsClearColor)
{
    constexpr std::uint32_t color = 0xFF123456u;

    const auto commands = CreateWindowCommands(
        std::make_shared<NeoWPF::UI::SolidColorBrush>(color));

    EXPECT_TRUE(commands.hasClearColor);
    EXPECT_EQ(commands.clearColor, color);
    EXPECT_TRUE(commands.commands.empty());
}

TEST(WindowBackgroundPaintTests, GradientBackgroundDrawsBeforeWindowContent)
{
    const auto commands = CreateWindowCommands(CreateGradient(0xFFFF0000u, 0xFF0000FFu));

    EXPECT_TRUE(commands.hasClearColor);
    EXPECT_EQ(commands.clearColor, 0u);
    ASSERT_EQ(commands.commands.size(), 1u);
    ASSERT_EQ(commands.commands.front().GetKind(),
              NeoWPF::Rendering::CommandList::CommandKind::Rect);
    const auto& background = commands.commands.front().GetRect();
    EXPECT_DOUBLE_EQ(background.x, 0.0);
    EXPECT_DOUBLE_EQ(background.y, 0.0);
    EXPECT_DOUBLE_EQ(background.w, 100.0);
    EXPECT_DOUBLE_EQ(background.h, 60.0);
    EXPECT_EQ(background.paint.kind, NeoWPF::Rendering::Paint::Kind::LinearGradient);
}

TEST(WindowBackgroundPaintTests, MissingBackgroundClearsToTransparent)
{
    const auto commands = CreateWindowCommands(nullptr);

    EXPECT_TRUE(commands.hasClearColor);
    EXPECT_EQ(commands.clearColor, 0u);
    EXPECT_TRUE(commands.commands.empty());
}

TEST(WindowBackgroundPaintTests, DifferentGradientBackgroundsHaveDifferentHashes)
{
    const auto first = CreateWindowCommands(CreateGradient(0xFFFF0000u, 0xFF0000FFu));
    const auto second = CreateWindowCommands(CreateGradient(0xFF00FF00u, 0xFFFFFF00u));

    EXPECT_NE(first.ComputeHash(), second.ComputeHash());
}

TEST(WindowBackgroundPaintTests, ZeroThicknessBorderDoesNotResolveGradientPaint)
{
    NeoWPF::UI::Border border;
    border.SetBorderThickness(0.0);
    border.SetBorderBrush(CreateGradient(0xFFFF0000u, 0xFF0000FFu));
    border.Arrange(100.0, 60.0);

    NeoWPF::Rendering::CommandList commands;
    border.BuildRenderCommands(commands);

    EXPECT_TRUE(commands.gradients.empty());
    EXPECT_TRUE(commands.gradientStops.empty());
}

}
