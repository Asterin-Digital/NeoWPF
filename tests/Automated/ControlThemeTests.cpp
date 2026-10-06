#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>

namespace {

class ControlThemeTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        NeoWPF::Core::Application::Initialize();
    }

    void TearDown() override
    {
        NeoWPF::Core::Application::Shutdown();
    }
};

bool ContainsRoundedRectColor(const NeoWPF::Rendering::CommandList& commands, std::uint32_t color)
{
    for (const auto& command : commands.commands) {
        if (command.kind == NeoWPF::Rendering::CommandList::CommandKind::RoundedRect &&
            command.roundedRect.color == color) {
            return true;
        }
    }
    return false;
}

TEST_F(ControlThemeTests, TextBlockTextBoxAndCheckboxFollowThemeAndPreserveLocalValues)
{
    using namespace NeoWPF::Core::ApplicationThemes;

    NeoWPF::UI::TextBlock textBlock(L"Theme");
    NeoWPF::UI::TextBlock styledTextBlock;
    NeoWPF::UI::TextBox textBox;
    NeoWPF::UI::Checkbox checkbox;
    ASSERT_TRUE(styledTextBlock.SetStylePropertyValue("FontSize", 17));

    ThemeDefinition customTheme;
    customTheme.Name = "Control Theme Test";
    customTheme.Palette[PaletteColor::Foreground] = 0xFF112233u;
    customTheme.Palette[PaletteColor::Surface] = 0xFF223344u;
    customTheme.Palette[PaletteColor::Border] = 0xFF334455u;
    customTheme.Palette[PaletteColor::Focus] = 0xFF445566u;
    customTheme.Palette[PaletteColor::Accent] = 0xFF556677u;
    customTheme.Palette[PaletteColor::AccentHover] = 0xFF667788u;
    customTheme.Palette[PaletteColor::AccentPressed] = 0xFF778899u;
    customTheme.Palette[PaletteColor::DisabledSurface] = 0xFF8899AAu;
    customTheme.FontSizes[ThemeFontSize::Control] = 20;
    customTheme.Paddings[ThemePadding::TextBlock] = NeoWPF::UI::Thickness(2, 3, 4, 5);
    customTheme.Paddings[ThemePadding::TextBox] = NeoWPF::UI::Thickness(6, 7, 8, 9);
    customTheme.Paddings[ThemePadding::Checkbox] = NeoWPF::UI::Thickness(10, 11, 12, 13);
    customTheme.CornerRadii[ThemeCornerRadius::Control] = NeoWPF::UI::CornerRadius(6);
    customTheme.CornerRadii[ThemeCornerRadius::Checkbox] = NeoWPF::UI::CornerRadius(3);

    auto& themeManager = ThemeManager::Instance();
    themeManager.RegisterTheme(customTheme);
    ASSERT_TRUE(themeManager.SetActiveTheme(customTheme.Name));

    const auto foreground = NeoWPF::UI::Color::FromArgb(0xFF112233u);
    const auto surface = NeoWPF::UI::Color::FromArgb(0xFF223344u);
    const auto border = NeoWPF::UI::Color::FromArgb(0xFF334455u);
    EXPECT_EQ(textBlock.GetTextColor(), foreground);
    EXPECT_EQ(textBlock.GetFontSize(), 20);
    EXPECT_EQ(textBlock.GetValueSource(NeoWPF::UI::TextBlock::FontSizeProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Theme);
    EXPECT_EQ(textBlock.GetPadding(), NeoWPF::UI::Thickness(2, 3, 4, 5));
    EXPECT_EQ(textBox.GetTextColor(), foreground);
    EXPECT_EQ(textBox.GetCaretColor(), foreground);
    EXPECT_EQ(textBox.GetBackgroundColor(), surface);
    EXPECT_EQ(textBox.GetBorderColor(), border);
    EXPECT_EQ(textBox.GetFontSize(), 20);
    EXPECT_EQ(textBox.GetPadding(), NeoWPF::UI::Thickness(6, 7, 8, 9));
    EXPECT_EQ(textBox.GetCornerRadius(), NeoWPF::UI::CornerRadius(6));
    EXPECT_EQ(checkbox.GetTextColor(), foreground);
    EXPECT_EQ(checkbox.GetBackgroundColor(), surface);
    EXPECT_EQ(checkbox.GetBorderColor(), border);
    EXPECT_EQ(checkbox.GetFontSize(), 20);
    EXPECT_EQ(checkbox.GetPadding(), NeoWPF::UI::Thickness(10, 11, 12, 13));
    EXPECT_EQ(checkbox.GetCornerRadius(), NeoWPF::UI::CornerRadius(3));
    EXPECT_EQ(styledTextBlock.GetFontSize(), 17);
    EXPECT_EQ(styledTextBlock.GetValueSource(NeoWPF::UI::TextBlock::FontSizeProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Style);

    textBlock.SetFontSize(25);
    textBox.SetPadding(NeoWPF::UI::Thickness(14));
    checkbox.SetFontSize(22);
    ASSERT_TRUE(checkbox.ApplyTemplate());
    checkbox.Initialize();
    checkbox.Arrange(120, 40);
    checkbox.SetChecked(true);

    ASSERT_TRUE(themeManager.SetActiveTheme("Light Mint"));

    EXPECT_EQ(textBlock.GetFontSize(), 25);
    EXPECT_EQ(textBox.GetPadding(), NeoWPF::UI::Thickness(14));
    EXPECT_EQ(checkbox.GetFontSize(), 22);
    EXPECT_EQ(textBox.GetTextColor(), NeoWPF::UI::Color::FromArgb(0xFF252925u));
    EXPECT_EQ(checkbox.GetTextColor(), NeoWPF::UI::Color::FromArgb(0xFF252925u));

    NeoWPF::Rendering::CommandList commands;
    checkbox.BuildRenderCommands(commands);
    EXPECT_TRUE(ContainsRoundedRectColor(commands, 0xFF708A74u));
}

}