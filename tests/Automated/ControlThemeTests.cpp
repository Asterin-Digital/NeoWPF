#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/ComboBox/ComboBox.h>
#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/UI/Controls/ProgressBar/ProgressBar.h>
#include <NeoWPF/UI/Controls/RadioButton/RadioButton.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Controls/Slider/Slider.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>

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

std::uint32_t GetSolidBrushColor(const std::shared_ptr<NeoWPF::UI::Brush>& brush)
{
    const auto solidBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(brush);
    return solidBrush ? solidBrush->GetArgb() : 0u;
}

TEST_F(ControlThemeTests, TextBlockTextBoxAndCheckboxFollowThemeAndPreserveLocalValues)
{
    using namespace NeoWPF::Core::ApplicationThemes;

    NeoWPF::UI::TextBlock textBlock(L"Theme");
    NeoWPF::UI::TextBlock styledTextBlock;
    NeoWPF::UI::TextBox textBox;
    NeoWPF::UI::Checkbox checkbox;
    NeoWPF::UI::RadioButton radioButton;
    NeoWPF::UI::ComboBox comboBox;
    NeoWPF::UI::ComboBoxItem comboBoxItem;
    NeoWPF::UI::ListBox listBox;
    NeoWPF::UI::ListBoxItem listBoxItem;
    NeoWPF::UI::ProgressBar progressBar;
    NeoWPF::UI::ScrollBar scrollBar;
    NeoWPF::UI::Slider slider;
    NeoWPF::UI::Border borderControl;
    NeoWPF::UI::Panel panel;
    NeoWPF::UI::Window window;
    ASSERT_TRUE(styledTextBlock.SetStylePropertyValue("FontSize", 17));

    ThemeDefinition customTheme;
    customTheme.Name = "Control Theme Test";
    customTheme.Palette[PaletteBrush::Foreground] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF112233u);
    customTheme.Palette[PaletteBrush::Background] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF102030u);
    customTheme.Palette[PaletteBrush::Surface] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF223344u);
    customTheme.Palette[PaletteBrush::Border] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF334455u);
    customTheme.Palette[PaletteBrush::Focus] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF445566u);
    customTheme.Palette[PaletteBrush::Accent] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF556677u);
    customTheme.Palette[PaletteBrush::AccentHover] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF667788u);
    customTheme.Palette[PaletteBrush::AccentPressed] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF778899u);
    customTheme.Palette[PaletteBrush::DisabledSurface] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF8899AAu);
    customTheme.Palette[PaletteBrush::Track] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF99AABBu);
    customTheme.Palette[PaletteBrush::Thumb] = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFFAABBCCu);
    customTheme.FontSizes[ThemeFontSize::Control] = 20;
    customTheme.Paddings[ThemePadding::TextBlock] = NeoWPF::UI::Thickness(2, 3, 4, 5);
    customTheme.Paddings[ThemePadding::TextBox] = NeoWPF::UI::Thickness(6, 7, 8, 9);
    customTheme.Paddings[ThemePadding::Checkbox] = NeoWPF::UI::Thickness(10, 11, 12, 13);
    customTheme.Paddings[ThemePadding::ComboBox] = NeoWPF::UI::Thickness(1, 2, 3, 4);
    customTheme.Paddings[ThemePadding::ComboBoxItem] = NeoWPF::UI::Thickness(5, 6, 7, 8);
    customTheme.Paddings[ThemePadding::ListBox] = NeoWPF::UI::Thickness(9, 10, 11, 12);
    customTheme.Paddings[ThemePadding::ListBoxItem] = NeoWPF::UI::Thickness(13, 14, 15, 16);
    customTheme.Paddings[ThemePadding::ProgressBar] = NeoWPF::UI::Thickness{};
    customTheme.Paddings[ThemePadding::RadioButton] = NeoWPF::UI::Thickness(17, 18, 19, 20);
    customTheme.Paddings[ThemePadding::ScrollBar] = NeoWPF::UI::Thickness{};
    customTheme.Paddings[ThemePadding::Slider] = NeoWPF::UI::Thickness{};
    customTheme.Metrics[ThemeMetric::ScrollBarThumbSize] = 31.0;
    customTheme.Metrics[ThemeMetric::ProgressBarBorderThickness] = 0.0;
    customTheme.Metrics[ThemeMetric::SliderBorderThickness] = 0.0;
    customTheme.Metrics[ThemeMetric::SliderThumbLength] = 21.0;
    customTheme.Paddings[ThemePadding::Border] = NeoWPF::UI::Thickness(3, 4, 5, 6);
    customTheme.BorderThicknesses[ThemeBorderThickness::Border] = NeoWPF::UI::Thickness(2, 3, 4, 5);
    customTheme.CornerRadii[ThemeCornerRadius::Control] = NeoWPF::UI::CornerRadius(6);
    customTheme.CornerRadii[ThemeCornerRadius::Checkbox] = NeoWPF::UI::CornerRadius(3);

    auto& themeManager = ThemeManager::Instance();
    themeManager.RegisterTheme(customTheme);
    ASSERT_TRUE(themeManager.SetActiveTheme(customTheme.Name));

    constexpr std::uint32_t foreground = 0xFF112233u;
    constexpr std::uint32_t surface = 0xFF223344u;
    constexpr std::uint32_t border = 0xFF334455u;
    EXPECT_EQ(GetSolidBrushColor(textBlock.GetForeground()), foreground);
    EXPECT_EQ(textBlock.GetFontSize(), 20);
    EXPECT_EQ(textBlock.GetValueSource(NeoWPF::UI::TextBlock::FontSizeProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Theme);
    EXPECT_EQ(textBlock.GetPadding(), NeoWPF::UI::Thickness(2, 3, 4, 5));
    EXPECT_EQ(GetSolidBrushColor(textBox.GetForeground()), foreground);
    EXPECT_EQ(GetSolidBrushColor(textBox.GetCaretBrush()), foreground);
    EXPECT_EQ(GetSolidBrushColor(textBox.GetBackgroundBrush()), surface);
    EXPECT_EQ(GetSolidBrushColor(textBox.GetBorderBrush()), border);
    EXPECT_EQ(textBox.GetFontSize(), 20);
    EXPECT_EQ(textBox.GetPadding(), NeoWPF::UI::Thickness(6, 7, 8, 9));
    EXPECT_EQ(textBox.GetCornerRadius(), NeoWPF::UI::CornerRadius(6));
    EXPECT_EQ(GetSolidBrushColor(checkbox.GetForeground()), foreground);
    EXPECT_EQ(GetSolidBrushColor(checkbox.GetBackgroundBrush()), surface);
    EXPECT_EQ(GetSolidBrushColor(checkbox.GetBorderBrush()), border);
    EXPECT_EQ(checkbox.GetFontSize(), 20);
    EXPECT_EQ(checkbox.GetPadding(), NeoWPF::UI::Thickness(10, 11, 12, 13));
    EXPECT_EQ(checkbox.GetCornerRadius(), NeoWPF::UI::CornerRadius(3));
    EXPECT_EQ(GetSolidBrushColor(radioButton.GetForeground()), foreground);
    EXPECT_EQ(GetSolidBrushColor(radioButton.GetBackgroundBrush()), surface);
    EXPECT_EQ(GetSolidBrushColor(radioButton.GetBorderBrush()), border);
    EXPECT_EQ(radioButton.GetFontSize(), 20);
    EXPECT_EQ(radioButton.GetPadding(), NeoWPF::UI::Thickness(17, 18, 19, 20));
    EXPECT_EQ(GetSolidBrushColor(comboBox.GetBackgroundBrush()), surface);
    EXPECT_EQ(GetSolidBrushColor(comboBox.GetBorderBrush()), border);
    EXPECT_EQ(comboBox.GetPadding(), NeoWPF::UI::Thickness(1, 2, 3, 4));
    EXPECT_EQ(comboBoxItem.GetPadding(), NeoWPF::UI::Thickness(5, 6, 7, 8));
    EXPECT_EQ(GetSolidBrushColor(listBox.GetBackgroundBrush()), surface);
    EXPECT_EQ(GetSolidBrushColor(listBox.GetBorderBrush()), border);
    EXPECT_EQ(listBox.GetPadding(), NeoWPF::UI::Thickness(9, 10, 11, 12));
    EXPECT_EQ(listBoxItem.GetPadding(), NeoWPF::UI::Thickness(13, 14, 15, 16));
    EXPECT_EQ(GetSolidBrushColor(progressBar.GetBackgroundBrush()), 0xFF99AABBu);
    EXPECT_DOUBLE_EQ(progressBar.GetBorderThickness(), 0.0);
    EXPECT_EQ(GetSolidBrushColor(scrollBar.GetTrackBrush()), 0xFF99AABBu);
    EXPECT_EQ(GetSolidBrushColor(scrollBar.GetThumbBrush()), 0xFFAABBCCu);
    EXPECT_DOUBLE_EQ(scrollBar.GetThumbSize(), 31.0);
    EXPECT_EQ(GetSolidBrushColor(slider.GetBackgroundBrush()), 0xFF99AABBu);
    EXPECT_DOUBLE_EQ(slider.GetBorderThickness(), 0.0);
    EXPECT_EQ(GetSolidBrushColor(borderControl.GetBackgroundBrush()), surface);
    EXPECT_EQ(GetSolidBrushColor(borderControl.GetBorderBrush()), border);
    EXPECT_EQ(borderControl.GetPadding(), NeoWPF::UI::Thickness(3, 4, 5, 6));
    EXPECT_EQ(borderControl.GetBorderThicknessValue(), NeoWPF::UI::Thickness(2, 3, 4, 5));
    EXPECT_EQ(borderControl.GetCornerRadius(), NeoWPF::UI::CornerRadius(6));
    EXPECT_EQ(panel.GetBackgroundBrush(), nullptr);
    EXPECT_EQ(GetSolidBrushColor(window.GetBackgroundBrush()), 0xFF102030u);
    EXPECT_EQ(styledTextBlock.GetFontSize(), 17);
    EXPECT_EQ(styledTextBlock.GetValueSource(NeoWPF::UI::TextBlock::FontSizeProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Style);

    textBlock.SetFontSize(25);
    textBox.SetPadding(NeoWPF::UI::Thickness(14));
    checkbox.SetFontSize(22);
    radioButton.SetFontSize(21);
    comboBox.SetPadding(NeoWPF::UI::Thickness(14));
    borderControl.SetPadding(NeoWPF::UI::Thickness(14));
    ASSERT_TRUE(checkbox.ApplyTemplate());
    checkbox.Initialize();
    checkbox.Arrange(120, 40);
    checkbox.SetChecked(true);

    ASSERT_TRUE(themeManager.SetActiveTheme("Light Mint"));

    EXPECT_EQ(textBlock.GetFontSize(), 25);
    EXPECT_EQ(textBox.GetPadding(), NeoWPF::UI::Thickness(14));
    EXPECT_EQ(checkbox.GetFontSize(), 22);
    EXPECT_EQ(radioButton.GetFontSize(), 21);
    EXPECT_EQ(comboBox.GetPadding(), NeoWPF::UI::Thickness(14));
    EXPECT_EQ(borderControl.GetPadding(), NeoWPF::UI::Thickness(14));
    EXPECT_EQ(GetSolidBrushColor(window.GetBackgroundBrush()), 0xFFF1F2EFu);
    EXPECT_EQ(GetSolidBrushColor(textBox.GetForeground()), 0xFF252925u);
    EXPECT_EQ(GetSolidBrushColor(checkbox.GetForeground()), 0xFF252925u);

    NeoWPF::Rendering::CommandList commands;
    checkbox.BuildRenderCommands(commands);
    EXPECT_TRUE(ContainsRoundedRectColor(commands, 0xFF708A74u));
}

}