#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>

#include <memory>

namespace {

TEST(PanelPaintTests, BuildRenderCommandsResolvesBackgroundWithPanelBounds)
{
    NeoWPF::UI::Panel panel;
    auto background = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF123456u);
    background->SetOpacity(0.5);
    panel.SetBackgroundBrush(background);
    panel.SetOpacity(0.25);
    panel.Arrange(64.0, 32.0);

    NeoWPF::Rendering::CommandList commands;
    panel.BuildRenderCommands(commands);

    ASSERT_EQ(commands.commands.size(), 1u);
    ASSERT_EQ(commands.commands.front().GetKind(),
              NeoWPF::Rendering::CommandList::CommandKind::RoundedRect);
    const auto& backgroundCommand = commands.commands.front().GetRoundedRect();
    EXPECT_DOUBLE_EQ(backgroundCommand.x, 0.0);
    EXPECT_DOUBLE_EQ(backgroundCommand.y, 0.0);
    EXPECT_DOUBLE_EQ(backgroundCommand.w, 64.0);
    EXPECT_DOUBLE_EQ(backgroundCommand.h, 32.0);
    EXPECT_EQ(backgroundCommand.paint.color, 0x7F123456u);
    EXPECT_FLOAT_EQ(backgroundCommand.opacity, 0.25f);
}

TEST(PanelPaintTests, BuildRenderCommandsSkipsTransparentBackground)
{
    NeoWPF::UI::Panel panel;
    panel.SetBackgroundBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0x00123456u));
    panel.Arrange(64.0, 32.0);

    NeoWPF::Rendering::CommandList commands;
    panel.BuildRenderCommands(commands);

    EXPECT_TRUE(commands.commands.empty());
}

TEST(PanelPaintTests, BuildRenderCommandsSkipsMissingBackground)
{
    NeoWPF::UI::Panel panel;
    panel.SetBackgroundBrush(nullptr);
    panel.Arrange(64.0, 32.0);

    NeoWPF::Rendering::CommandList commands;
    panel.BuildRenderCommands(commands);

    EXPECT_TRUE(commands.commands.empty());
}

}