#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/UI/Controls/ProgressRing/ProgressRing.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

#include <memory>

namespace {

constexpr std::uint32_t kBrushColor = 0xFF123456u;
constexpr std::uint32_t kResolvedColor = 0x7F123456u;

class TestTextBox final : public NeoWPF::UI::TextBox {
public:
    using NeoWPF::UI::TextBox::BuildRenderCommands;
};

std::shared_ptr<NeoWPF::UI::SolidColorBrush> MakeBrush()
{
    auto brush = std::make_shared<NeoWPF::UI::SolidColorBrush>(kBrushColor);
    brush->SetOpacity(0.5);
    return brush;
}

const NeoWPF::Rendering::CommandList::RoundedRectData& GetRoundedRect(
    const NeoWPF::Rendering::CommandList& commands,
    std::size_t index = 0)
{
    static const NeoWPF::Rendering::CommandList::RoundedRectData empty{};
    if (index >= commands.commands.size() ||
        commands.commands[index].GetKind() !=
            NeoWPF::Rendering::CommandList::CommandKind::RoundedRect) {
        ADD_FAILURE() << "Expected a rounded-rectangle command at index " << index;
        return empty;
    }
    return commands.commands.at(index).GetRoundedRect();
}

TEST(ControlPaintTests, ButtonResolvesItsBrushes)
{
    NeoWPF::UI::Button button;
    button.SetTemplate(nullptr);
    button.SetBorderBrush(MakeBrush());
    button.SetBackgroundBrush(MakeBrush());
    button.Arrange(40.0, 24.0);

    NeoWPF::Rendering::CommandList commands;
    button.BuildRenderCommands(commands);

    ASSERT_GE(commands.commands.size(), 2u);
    EXPECT_EQ(GetRoundedRect(commands).paint.color, kResolvedColor);
    EXPECT_EQ(GetRoundedRect(commands, 1).paint.color, kResolvedColor);
}

TEST(ControlPaintTests, CheckboxResolvesItsBrushes)
{
    NeoWPF::UI::Checkbox checkbox;
    checkbox.SetTemplate(nullptr);
    checkbox.SetBackgroundBrush(MakeBrush());
    checkbox.SetBorderBrush(MakeBrush());
    checkbox.Arrange(40.0, 24.0);

    NeoWPF::Rendering::CommandList commands;
    checkbox.BuildRenderCommands(commands);

    ASSERT_GE(commands.commands.size(), 2u);
    EXPECT_EQ(GetRoundedRect(commands).paint.color, kResolvedColor);
    EXPECT_EQ(GetRoundedRect(commands, 1).paint.color, kResolvedColor);
}

TEST(ControlPaintTests, ListBoxItemResolvesSelectionBrushes)
{
    NeoWPF::UI::ListBoxItem item;
    item.SetBorderBrush(MakeBrush());
    item.SetBackgroundBrush(MakeBrush());
    item.SetIsSelected(true);
    item.Arrange(40.0, 24.0);

    NeoWPF::Rendering::CommandList commands;
    item.BuildRenderCommands(commands);

    ASSERT_GE(commands.commands.size(), 2u);
    EXPECT_EQ(GetRoundedRect(commands).paint.color, kResolvedColor);
    EXPECT_EQ(GetRoundedRect(commands, 1).paint.color, kResolvedColor);
}

TEST(ControlPaintTests, ProgressRingResolvesTrackBrushOpacityOnce)
{
    NeoWPF::UI::ProgressRing ring;
    ring.SetBackgroundBrush(MakeBrush());
    ring.SetOpacity(0.5);
    ring.Arrange(48.0, 48.0);

    NeoWPF::Rendering::CommandList commands;
    ring.BuildRenderCommands(commands);

    ASSERT_FALSE(commands.commands.empty());
    const auto& arc = commands.commands.front().GetArc();
    EXPECT_EQ(arc.paint.color, kResolvedColor);
    EXPECT_FLOAT_EQ(arc.opacity, 0.5f);
}

TEST(ControlPaintTests, ScrollBarResolvesTrackBrush)
{
    NeoWPF::UI::ScrollBar scrollBar;
    scrollBar.SetTemplate(nullptr);
    scrollBar.SetTrackBrush(MakeBrush());
    scrollBar.SetContentSize(100.0);
    scrollBar.SetViewportSize(20.0);
    scrollBar.Arrange(12.0, 100.0);

    NeoWPF::Rendering::CommandList commands;
    scrollBar.BuildRenderCommands(commands);

    ASSERT_FALSE(commands.commands.empty());
    EXPECT_EQ(GetRoundedRect(commands).paint.color, kResolvedColor);
}

TEST(ControlPaintTests, TemplateVisualResolvesFillAndBorderBrushes)
{
    NeoWPF::UI::Templates::TemplateVisual visual;
    visual.SetRect(0.0, 0.0, 40.0, 24.0);
    visual.SetBorderThickness(1);
    visual.SetBorderBrush(MakeBrush());
    visual.SetFillBrush(MakeBrush());

    NeoWPF::Rendering::CommandList commands;
    visual.BuildRenderCommands(commands);

    ASSERT_EQ(commands.commands.size(), 2u);
    EXPECT_EQ(GetRoundedRect(commands).paint.color, kResolvedColor);
    EXPECT_EQ(GetRoundedRect(commands, 1).paint.color, kResolvedColor);
}

TEST(ControlPaintTests, TextBoxResolvesBorderAndBackgroundBrushes)
{
    TestTextBox textBox;
    textBox.SetTemplate(nullptr);
    textBox.SetBorderBrush(MakeBrush());
    textBox.SetBackgroundBrush(MakeBrush());
    textBox.Arrange(40.0, 24.0);

    NeoWPF::Rendering::CommandList commands;
    textBox.BuildRenderCommands(commands);

    ASSERT_EQ(commands.commands.size(), 2u);
    EXPECT_EQ(GetRoundedRect(commands).paint.color, kResolvedColor);
    EXPECT_EQ(GetRoundedRect(commands, 1).paint.color, kResolvedColor);
}

TEST(ControlPaintTests, ThumbResolvesBackgroundBrush)
{
    NeoWPF::UI::Thumb thumb;
    thumb.SetBackgroundBrush(MakeBrush());
    thumb.SetBounds(0, 0, 40, 24);

    NeoWPF::Rendering::CommandList commands;
    thumb.BuildRenderCommands(commands);

    ASSERT_EQ(commands.commands.size(), 1u);
    EXPECT_EQ(GetRoundedRect(commands).paint.color, kResolvedColor);
}

} // namespace
