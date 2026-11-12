#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/ToggleSwitch/ToggleSwitch.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

void RaisePointerEvent(NeoWPF::UI::ToggleSwitch& toggleSwitch,
                       const NeoWPF::Input::Core::RoutedEvent& routedEvent,
                       NeoWPF::Input::Core::PointerPhase phase,
                       std::uint32_t changedButton = 0,
                       std::uint32_t buttons = 0)
{
    NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse, phase,
        0.0, 0.0, buttons, changedButton);
    NeoWPF::Input::Core::RoutedPointerEventArgs args(routedEvent, packet);
    args.SetSource(&toggleSwitch);
    args.SetOriginalSource(&toggleSwitch);
    toggleSwitch.RaiseEvent(&args);
}

using RoundedRectData = decltype(
    std::declval<NeoWPF::Rendering::CommandList::Command>().roundedRect);

class ToggleSwitchTests : public ::testing::Test {
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

std::vector<RoundedRectData> GetRoundedRectCommands(
    const NeoWPF::Rendering::CommandList& commands)
{
    std::vector<RoundedRectData> rectangles;
    for (const auto& command : commands.commands) {
        if (command.kind == NeoWPF::Rendering::CommandList::CommandKind::RoundedRect) {
            rectangles.push_back(command.roundedRect);
        }
    }
    return rectangles;
}

std::uint32_t GetBrushColor(const std::shared_ptr<NeoWPF::UI::Brush>& brush)
{
    const auto solidBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(brush);
    return solidBrush ? solidBrush->GetArgb() : 0u;
}

std::uint32_t GetRenderedTrackColor(NeoWPF::UI::ToggleSwitch& toggleSwitch)
{
    NeoWPF::Rendering::CommandList commands;
    toggleSwitch.BuildRenderCommands(commands);
    const auto rectangles = GetRoundedRectCommands(commands);
    return rectangles.empty() ? 0u : rectangles.front().paint.color;
}

}

TEST_F(ToggleSwitchTests, DefaultsToUncheckedAndSynchronizesItsDependencyProperty)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;

    EXPECT_FALSE(toggleSwitch.IsChecked());
    EXPECT_FALSE(toggleSwitch.GetValue<bool>(NeoWPF::UI::ToggleSwitch::IsCheckedProperty()));

    toggleSwitch.SetChecked(true);

    EXPECT_TRUE(toggleSwitch.IsChecked());
    EXPECT_TRUE(toggleSwitch.GetValue<bool>(NeoWPF::UI::ToggleSwitch::IsCheckedProperty()));
}

TEST_F(ToggleSwitchTests, CheckedAndUncheckedEventsOnlyFireForStateChanges)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    int checkedCount = 0;
    int uncheckedCount = 0;
    toggleSwitch.Checked += [&checkedCount] { ++checkedCount; };
    toggleSwitch.Unchecked += [&uncheckedCount] { ++uncheckedCount; };

    toggleSwitch.SetChecked(true);
    toggleSwitch.SetChecked(true);
    toggleSwitch.SetChecked(false);
    toggleSwitch.SetChecked(false);

    EXPECT_EQ(checkedCount, 1);
    EXPECT_EQ(uncheckedCount, 1);
}

TEST_F(ToggleSwitchTests, ToggleChangesCheckedState)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;

    toggleSwitch.Toggle();
    EXPECT_TRUE(toggleSwitch.IsChecked());

    toggleSwitch.Toggle();
    EXPECT_FALSE(toggleSwitch.IsChecked());
}

TEST_F(ToggleSwitchTests, PointerActivationRaisesClickAndTogglesOnce)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    int clickCount = 0;
    toggleSwitch.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_TRUE(toggleSwitch.IsChecked());
    EXPECT_EQ(clickCount, 1);
}

TEST_F(ToggleSwitchTests, PointerReleaseWithoutPressDoesNotToggle)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    int clickCount = 0;
    toggleSwitch.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up);

    EXPECT_FALSE(toggleSwitch.IsChecked());
    EXPECT_EQ(clickCount, 0);
}

TEST_F(ToggleSwitchTests, PointerLeaveCancelsPendingToggle)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    int clickCount = 0;
    toggleSwitch.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerExitedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Leave);
    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_FALSE(toggleSwitch.IsChecked());
    EXPECT_EQ(clickCount, 0);
}

TEST_F(ToggleSwitchTests, DisabledPointerActivationPreservesStateAndDoesNotRaiseClick)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.SetEnabled(false);
    int clickCount = 0;
    toggleSwitch.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_FALSE(toggleSwitch.IsChecked());
    EXPECT_EQ(clickCount, 0);
}

TEST_F(ToggleSwitchTests, CheckedStateUsesTheThemeAccentTrack)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.ApplyTemplate();
    toggleSwitch.Initialize();
    toggleSwitch.Arrange(36.0, 20.0);
    toggleSwitch.SetChecked(true);

    NeoWPF::Rendering::CommandList commands;
    toggleSwitch.BuildRenderCommands(commands);
    const auto rectangles = GetRoundedRectCommands(commands);
    ASSERT_EQ(rectangles.size(), 2u);

    EXPECT_EQ(rectangles[0].paint.color, GetBrushColor(
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Accent)));
}

TEST_F(ToggleSwitchTests, DisabledStateUsesTheThemeDisabledSurface)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.SetEnabled(false);
    toggleSwitch.ApplyTemplate();
    toggleSwitch.Initialize();
    toggleSwitch.Arrange(36.0, 20.0);

    NeoWPF::Rendering::CommandList commands;
    toggleSwitch.BuildRenderCommands(commands);
    const auto rectangles = GetRoundedRectCommands(commands);
    ASSERT_EQ(rectangles.size(), 2u);

    EXPECT_EQ(rectangles[0].paint.color, GetBrushColor(
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::DisabledSurface)));
}

TEST_F(ToggleSwitchTests, PointerVisualStatesUseThemePaletteRoles)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.ApplyTemplate();
    toggleSwitch.Initialize();
    toggleSwitch.Arrange(36.0, 20.0);
    using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;

    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch),
              GetBrushColor(GetPaletteBrush(PaletteBrush::Track)));

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerEnteredEvent(),
                      NeoWPF::Input::Core::PointerPhase::Enter);
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch),
              GetBrushColor(GetPaletteBrush(PaletteBrush::AccentHover)));

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch),
              GetBrushColor(GetPaletteBrush(PaletteBrush::AccentPressed)));

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);
    ASSERT_TRUE(toggleSwitch.IsChecked());
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch),
              GetBrushColor(GetPaletteBrush(PaletteBrush::Accent)));

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerExitedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Leave);
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch),
              GetBrushColor(GetPaletteBrush(PaletteBrush::Accent)));

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerEnteredEvent(),
                      NeoWPF::Input::Core::PointerPhase::Enter);
    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch),
              GetBrushColor(GetPaletteBrush(PaletteBrush::Accent)));
}

TEST_F(ToggleSwitchTests, PointerStateChangesInvalidateTheVisualTree)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.ApplyTemplate();
    toggleSwitch.Initialize();
    toggleSwitch.Arrange(36.0, 20.0);

    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerEnteredEvent(),
                      NeoWPF::Input::Core::PointerPhase::Enter);
    toggleSwitch.ClearDirty();
    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerExitedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Leave);

    EXPECT_TRUE(toggleSwitch.IsTreeDirty());
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch), GetBrushColor(
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Track)));

    toggleSwitch.SetChecked(true);
    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerEnteredEvent(),
                      NeoWPF::Input::Core::PointerPhase::Enter);
    toggleSwitch.ClearDirty();
    RaisePointerEvent(toggleSwitch, NeoWPF::Input::Core::PointerExitedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Leave);

    EXPECT_TRUE(toggleSwitch.IsTreeDirty());
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch), GetBrushColor(
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Accent)));
}

TEST_F(ToggleSwitchTests, ContentTextUpdatesTheLabelAndDesiredSize)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.Measure(200.0, 40.0);
    const double emptyWidth = toggleSwitch.GetDesiredWidth();

    toggleSwitch.SetContentText(L"Wi-Fi");
    toggleSwitch.Measure(200.0, 40.0);

    const auto label = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(
        toggleSwitch.GetContentElement());
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(label->GetText(), L"Wi-Fi");
    EXPECT_EQ(toggleSwitch.GetValue<std::wstring>(
                  NeoWPF::UI::ToggleSwitch::ContentTextProperty()), L"Wi-Fi");
    EXPECT_GT(toggleSwitch.GetDesiredWidth(), emptyWidth);
}

TEST_F(ToggleSwitchTests, ClearingContentTextRestoresSwitchOnlySizing)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.Measure(200.0, 40.0);
    const double switchOnlyWidth = toggleSwitch.GetDesiredWidth();

    toggleSwitch.SetContentText(L"Wi-Fi");
    toggleSwitch.Measure(200.0, 40.0);
    ASSERT_GT(toggleSwitch.GetDesiredWidth(), switchOnlyWidth);

    toggleSwitch.SetContentText(L"");
    toggleSwitch.Measure(200.0, 40.0);

    const auto label = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(
        toggleSwitch.GetContentElement());
    ASSERT_NE(label, nullptr);
    EXPECT_TRUE(label->GetText().empty());
    EXPECT_DOUBLE_EQ(toggleSwitch.GetDesiredWidth(), switchOnlyWidth);
}

TEST_F(ToggleSwitchTests, CheckedStateMovesThumbAcrossTheTrack)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.ApplyTemplate();
    toggleSwitch.Initialize();
    toggleSwitch.Arrange(100.0, 24.0);

    NeoWPF::Rendering::CommandList uncheckedCommands;
    toggleSwitch.BuildRenderCommands(uncheckedCommands);
    const auto uncheckedRects = GetRoundedRectCommands(uncheckedCommands);
    ASSERT_EQ(uncheckedRects.size(), 2u);

    toggleSwitch.SetChecked(true);
    NeoWPF::Rendering::CommandList checkedCommands;
    toggleSwitch.BuildRenderCommands(checkedCommands);
    const auto checkedRects = GetRoundedRectCommands(checkedCommands);
    ASSERT_EQ(checkedRects.size(), 2u);

    EXPECT_DOUBLE_EQ(uncheckedRects[0].x, checkedRects[0].x);
    EXPECT_GT(checkedRects[1].x, uncheckedRects[1].x);
}

TEST_F(ToggleSwitchTests, ThemeChangesUpdateTheLabelForeground)
{
    auto& themeManager = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
    const auto originalTheme = themeManager.GetActiveThemeName();
    ASSERT_TRUE(themeManager.SetActiveTheme("Light Mint"));
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.ApplyTemplate();
    toggleSwitch.Initialize();
    toggleSwitch.Arrange(100.0, 24.0);
    auto label = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(
        toggleSwitch.GetContentElement());
    ASSERT_NE(label, nullptr);

    ASSERT_TRUE(themeManager.SetActiveTheme("Dark Mint"));
    EXPECT_EQ(GetBrushColor(label->GetForeground()),
              GetBrushColor(themeManager.GetActiveTheme()->Palette.at(
                  NeoWPF::Core::ApplicationThemes::PaletteBrush::Foreground)));
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch), GetBrushColor(
        themeManager.GetActiveTheme()->Palette.at(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Track)));

    ASSERT_TRUE(themeManager.SetActiveTheme(originalTheme));
}

TEST_F(ToggleSwitchTests, ThemeSelectedBeforeInitializationStylesTheTrack)
{
    auto& themeManager = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
    const auto originalTheme = themeManager.GetActiveThemeName();
    ASSERT_TRUE(themeManager.SetActiveTheme("Light Mint"));

    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.ApplyTemplate();
    toggleSwitch.Arrange(36.0, 20.0);
    ASSERT_TRUE(themeManager.SetActiveTheme("Dark Mint"));
    toggleSwitch.Initialize();

    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch), GetBrushColor(
        themeManager.GetActiveTheme()->Palette.at(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Track)));

    ASSERT_TRUE(themeManager.SetActiveTheme(originalTheme));
}

TEST_F(ToggleSwitchTests, XamlRegistrationCreatesTheControlWithProperties)
{
    const auto* type = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find("ToggleSwitch");
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->CppTypeName, "NeoWPF::UI::ToggleSwitch");
    EXPECT_EQ(type->HeaderPath, "NeoWPF/UI/Controls/ToggleSwitch/ToggleSwitch.h");

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<ToggleSwitch ContentText="Wi-Fi" IsChecked="True" />)");
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    const auto* toggleSwitch = dynamic_cast<const NeoWPF::UI::ToggleSwitch*>(tree.get());

    ASSERT_NE(toggleSwitch, nullptr);
    EXPECT_EQ(toggleSwitch->GetContentText(), L"Wi-Fi");
    EXPECT_TRUE(toggleSwitch->IsChecked());
}

TEST_F(ToggleSwitchTests, FocusChangesPreserveCheckedVisualState)
{
    NeoWPF::UI::ToggleSwitch toggleSwitch;
    toggleSwitch.ApplyTemplate();
    toggleSwitch.Initialize();
    toggleSwitch.Arrange(36.0, 20.0);
    toggleSwitch.SetChecked(true);

    const auto accentColor = GetBrushColor(
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Accent));

    toggleSwitch.OnGotFocus();
    EXPECT_TRUE(toggleSwitch.IsChecked());
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch), accentColor);

    toggleSwitch.OnLostFocus();
    EXPECT_TRUE(toggleSwitch.IsChecked());
    EXPECT_EQ(GetRenderedTrackColor(toggleSwitch), accentColor);
}