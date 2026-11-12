#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Input/Command.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>

#include <any>
#include <memory>
#include <string>

namespace {

void RaisePointerEvent(NeoWPF::UI::UIElement& element,
                       const NeoWPF::Input::Core::RoutedEvent& routedEvent,
                       NeoWPF::Input::Core::PointerPhase phase,
                       std::uint32_t changedButton = 0,
                       std::uint32_t buttons = 0)
{
    NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse, phase,
        10.0, 10.0, buttons, changedButton);
    NeoWPF::Input::Core::RoutedPointerEventArgs args(routedEvent, packet);
    args.SetSource(&element);
    args.SetOriginalSource(&element);
    element.RaiseEvent(&args);
}

class ButtonTests : public ::testing::Test {
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

std::shared_ptr<NeoWPF::UI::Button> MakeSizedButton()
{
    auto button = std::make_shared<NeoWPF::UI::Button>();
    button->SetWidth(120.0);
    button->SetHeight(40.0);
    button->ApplyTemplate();
    button->Initialize();
    button->Measure(120.0, 40.0);
    button->Arrange(120.0, 40.0);
    return button;
}

bool ContainsRoundedRectColor(const NeoWPF::Rendering::CommandList& commands, std::uint32_t color)
{
    for (const auto& command : commands.commands) {
        if (command.kind == NeoWPF::Rendering::CommandList::CommandKind::RoundedRect &&
            command.roundedRect.paint.color == color) {
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

class RecordingCommand final : public NeoWPF::Input::ICommand {
public:
    explicit RecordingCommand(bool canExecute = true)
        : m_canExecute(canExecute)
    {
    }

    bool CanExecute(const std::any&) const override
    {
        return m_canExecute;
    }

    void Execute(const std::any& parameter) override
    {
        ++executeCount;
        receivedParameter = parameter;
    }

    void AddCanExecuteChangedHandler(const std::function<void()>&) override {}
    void RemoveCanExecuteChangedHandler(const std::function<void()>&) override {}
    void RaiseCanExecuteChanged() override {}

    int executeCount = 0;
    std::any receivedParameter;

private:
    bool m_canExecute;
};

TEST_F(ButtonTests, ExposesCompleteStaticPropertyContract)
{
    const auto& text = NeoWPF::UI::Button::TextProperty();
    const auto& foreground = NeoWPF::UI::Button::ForegroundProperty();
    const auto& fontSize = NeoWPF::UI::Button::FontSizeProperty();
    const auto& fontFamily = NeoWPF::UI::Button::FontFamilyProperty();
    const auto& fontWeight = NeoWPF::UI::Button::FontWeightProperty();
    const auto& fontStyle = NeoWPF::UI::Button::FontStyleProperty();
    const auto& horizontal = NeoWPF::UI::Button::TextHorizontalAlignmentProperty();
    const auto& vertical = NeoWPF::UI::Button::TextVerticalAlignmentProperty();

    EXPECT_EQ(foreground.GetPropertyType(), std::type_index(typeid(std::shared_ptr<NeoWPF::UI::Brush>)));
    EXPECT_EQ(text.GetName(), "Text");
    EXPECT_EQ(foreground.GetName(), "Foreground");
    EXPECT_EQ(fontSize.GetName(), "FontSize");
    EXPECT_EQ(fontFamily.GetName(), "FontFamily");
    EXPECT_EQ(fontWeight.GetName(), "FontWeight");
    EXPECT_EQ(fontStyle.GetName(), "FontStyle");
    EXPECT_EQ(horizontal.GetName(), "TextHorizontalAlignment");
    EXPECT_EQ(vertical.GetName(), "TextVerticalAlignment");
    EXPECT_EQ(text.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Button)));
}

TEST_F(ButtonTests, DirectSettersKeepAllDependencyPropertiesAndGettersSynchronized)
{
    NeoWPF::UI::Button button;
    const auto textColor = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF112233u);

    button.SetText(L"Save");
    button.SetForeground(textColor);
    button.SetFontSize(21);
    button.SetFontFamily(L"Test Sans");
    button.SetFontWeight(700);
    button.SetFontStyle(NeoWPF::UI::FontStyle::Italic);
    button.SetTextHorizontalAlignment(NeoWPF::UI::TextHorizontalAlignment::Right);
    button.SetTextVerticalAlignment(NeoWPF::UI::TextVerticalAlignment::Bottom);
    button.SetPadding(NeoWPF::UI::Thickness(1, 2, 3, 4));
    button.SetBackgroundBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF445566u));
    button.SetBorderBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF778899u));
    button.SetBorderThickness(3.0);

    EXPECT_EQ(button.GetValue<std::wstring>(NeoWPF::UI::Button::TextProperty()), L"Save");
    EXPECT_EQ(button.GetValue<std::shared_ptr<NeoWPF::UI::Brush>>(NeoWPF::UI::Button::ForegroundProperty()), textColor);
    EXPECT_EQ(button.GetValue<int>(NeoWPF::UI::Button::FontSizeProperty()), 21);
    EXPECT_EQ(button.GetValue<std::wstring>(NeoWPF::UI::Button::FontFamilyProperty()), L"Test Sans");
    EXPECT_EQ(button.GetValue<int>(NeoWPF::UI::Button::FontWeightProperty()), 700);
    EXPECT_EQ(button.GetValue<NeoWPF::UI::FontStyle>(NeoWPF::UI::Button::FontStyleProperty()), NeoWPF::UI::FontStyle::Italic);
    EXPECT_EQ(button.GetValue<NeoWPF::UI::TextHorizontalAlignment>(NeoWPF::UI::Button::TextHorizontalAlignmentProperty()), NeoWPF::UI::TextHorizontalAlignment::Right);
    EXPECT_EQ(button.GetValue<NeoWPF::UI::TextVerticalAlignment>(NeoWPF::UI::Button::TextVerticalAlignmentProperty()), NeoWPF::UI::TextVerticalAlignment::Bottom);
    EXPECT_EQ(button.GetPadding(), NeoWPF::UI::Thickness(1, 2, 3, 4));
    EXPECT_EQ(GetSolidBrushColor(button.GetBackgroundBrush()), 0xFF445566u);
    EXPECT_EQ(GetSolidBrushColor(button.GetBorderBrush()), 0xFF778899u);
    EXPECT_DOUBLE_EQ(button.GetBorderThickness(), 3.0);

    EXPECT_EQ(button.GetText(), L"Save");
    EXPECT_EQ(button.GetForeground(), textColor);
    EXPECT_EQ(button.GetFontSize(), 21);
    EXPECT_EQ(button.GetFontFamily(), L"Test Sans");
    EXPECT_EQ(button.GetFontWeight(), 700);
    EXPECT_EQ(button.GetFontStyle(), NeoWPF::UI::FontStyle::Italic);
    EXPECT_EQ(button.GetTextHorizontalAlignment(), NeoWPF::UI::TextHorizontalAlignment::Right);
    EXPECT_EQ(button.GetTextVerticalAlignment(), NeoWPF::UI::TextVerticalAlignment::Bottom);
}

TEST_F(ButtonTests, StyleValuesAreVisibleAndLocalValuesWinThenClearBackToStyle)
{
    NeoWPF::UI::Button button;
    const std::shared_ptr<NeoWPF::UI::Brush> background =
        std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF102030u);
    const std::shared_ptr<NeoWPF::UI::Brush> localBackground =
        std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF405060u);
    const std::shared_ptr<NeoWPF::UI::Brush> borderBrush =
        std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF708090u);

    ASSERT_TRUE(button.SetStylePropertyValue("Background", background));
    ASSERT_TRUE(button.SetStylePropertyValue("BorderBrush", borderBrush));
    ASSERT_TRUE(button.SetStylePropertyValue("FontSize", 19));

    EXPECT_EQ(button.GetBackgroundBrush(), background);
    EXPECT_EQ(GetSolidBrushColor(button.GetBorderBrush()), 0xFF708090u);
    EXPECT_EQ(button.GetFontSize(), 19);
    EXPECT_EQ(button.GetValueSource(NeoWPF::UI::Control::BackgroundProperty()), NeoWPF::UI::Properties::PropertyValueSource::Style);

    button.SetBackgroundBrush(localBackground);
    EXPECT_EQ(button.GetBackgroundBrush(), localBackground);
    EXPECT_EQ(button.GetValueSource(NeoWPF::UI::Control::BackgroundProperty()), NeoWPF::UI::Properties::PropertyValueSource::Local);

    button.ClearValue(NeoWPF::UI::Control::BackgroundProperty());
    EXPECT_EQ(button.GetBackgroundBrush(), background);
    EXPECT_EQ(button.GetValueSource(NeoWPF::UI::Control::BackgroundProperty()), NeoWPF::UI::Properties::PropertyValueSource::Style);
}

TEST_F(ButtonTests, TemplateRegistersExpectedParts)
{
    NeoWPF::UI::Button button;

    ASSERT_TRUE(button.HasTemplate());
    ASSERT_TRUE(button.ApplyTemplate());
    EXPECT_TRUE(button.HasTemplateChild("PART_Background"));
    EXPECT_TRUE(button.HasTemplateChild("PART_ContentHost"));
    EXPECT_EQ(button.GetTemplateChildNames().size(), 2u);
    EXPECT_NE(button.FindTemplateChild("PART_Background"), nullptr);
    EXPECT_NE(button.FindTemplateChild("PART_ContentHost"), nullptr);
}

TEST_F(ButtonTests, UsesSharedPaletteForLightAndDarkThemes)
{
    auto& themeManager = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
    EXPECT_EQ(themeManager.GetActiveThemeName(), "Light Mint");

    std::shared_ptr<NeoWPF::UI::Brush> surface;
    std::shared_ptr<NeoWPF::UI::Brush> foreground;
    std::shared_ptr<NeoWPF::UI::Brush> border;
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface, surface));
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Foreground, foreground));
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Border, border));

    auto lightButton = std::make_shared<NeoWPF::UI::Button>();
    EXPECT_EQ(GetSolidBrushColor(lightButton->GetBackgroundBrush()), GetSolidBrushColor(surface));
    EXPECT_EQ(GetSolidBrushColor(lightButton->GetForeground()), GetSolidBrushColor(foreground));
    EXPECT_EQ(GetSolidBrushColor(lightButton->GetBorderBrush()), GetSolidBrushColor(border));

    ASSERT_TRUE(themeManager.SetActiveTheme("Dark Mint"));
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface, surface));
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Foreground, foreground));
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Border, border));

    auto darkButton = std::make_shared<NeoWPF::UI::Button>();
    EXPECT_EQ(GetSolidBrushColor(darkButton->GetBackgroundBrush()), GetSolidBrushColor(surface));
    EXPECT_EQ(GetSolidBrushColor(darkButton->GetForeground()), GetSolidBrushColor(foreground));
    EXPECT_EQ(GetSolidBrushColor(darkButton->GetBorderBrush()), GetSolidBrushColor(border));
}

TEST_F(ButtonTests, ExistingButtonUpdatesWhenThemeIsApplied)
{
    auto& themeManager = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
    NeoWPF::UI::Button button(L"Theme test");
    const NeoWPF::UI::Thickness customPadding(4, 6, 8, 10);
    ASSERT_TRUE(button.SetStylePropertyValue("Padding", customPadding));
    ASSERT_TRUE(button.SetStylePropertyValue("FontSize", 19));

    ASSERT_TRUE(themeManager.ApplyTheme("Dark Mint"));

    std::shared_ptr<NeoWPF::UI::Brush> expectedSurface;
    std::shared_ptr<NeoWPF::UI::Brush> expectedForeground;
    std::shared_ptr<NeoWPF::UI::Brush> expectedBorder;
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface, expectedSurface));
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Foreground, expectedForeground));
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Border, expectedBorder));

    EXPECT_EQ(GetSolidBrushColor(button.GetBackgroundBrush()), GetSolidBrushColor(expectedSurface));
    EXPECT_EQ(GetSolidBrushColor(button.GetForeground()), GetSolidBrushColor(expectedForeground));
    EXPECT_EQ(GetSolidBrushColor(button.GetBorderBrush()), GetSolidBrushColor(expectedBorder));
    EXPECT_EQ(button.GetPadding(), customPadding);
    EXPECT_EQ(button.GetFontSize(), 19);
}

TEST_F(ButtonTests, ExistingButtonRefreshesItsVisualStateWhenThemeChanges)
{
    auto& themeManager = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
    auto button = MakeSizedButton();

    NeoWPF::Rendering::CommandList lightCommands;
    button->BuildRenderCommands(lightCommands);
    EXPECT_TRUE(ContainsRoundedRectColor(lightCommands, 0xFFF8F9F7u));

    ASSERT_TRUE(themeManager.SetActiveTheme("Dark Mint"));

    NeoWPF::Rendering::CommandList darkCommands;
    button->BuildRenderCommands(darkCommands);
    EXPECT_TRUE(ContainsRoundedRectColor(darkCommands, 0xFF1D1D1Du));
    EXPECT_TRUE(ContainsRoundedRectColor(darkCommands, 0xFF686868u));
}

TEST_F(ButtonTests, ActiveThemeUpdatesFontSizeAndButtonPadding)
{
    auto& themeManager = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
    NeoWPF::UI::Button button(L"Theme metrics");

    EXPECT_EQ(button.GetFontSize(), 14);
    EXPECT_EQ(button.GetPadding(), NeoWPF::UI::Thickness(8));

    NeoWPF::Core::ApplicationThemes::ThemeDefinition largeControls;
    largeControls.Name = "Large Controls";
    largeControls.FontSizes[NeoWPF::Core::ApplicationThemes::ThemeFontSize::Control] = 18;
    largeControls.Paddings[NeoWPF::Core::ApplicationThemes::ThemePadding::Button] =
        NeoWPF::UI::Thickness(12, 10, 12, 10);
    themeManager.RegisterTheme(largeControls);

    ASSERT_TRUE(themeManager.SetActiveTheme("Large Controls"));
    EXPECT_EQ(button.GetFontSize(), 18);
    EXPECT_EQ(button.GetPadding(), NeoWPF::UI::Thickness(12, 10, 12, 10));

    ASSERT_TRUE(themeManager.SetActiveTheme("Light Mint"));
    EXPECT_EQ(button.GetFontSize(), 14);
    EXPECT_EQ(button.GetPadding(), NeoWPF::UI::Thickness(8));
}

TEST_F(ButtonTests, NormalStateUsesDependencyPropertyColorsAndHonorsBorderThickness)
{
    auto button = MakeSizedButton();
    button->SetBackgroundBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF112233u));
    button->SetBorderBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF445566u));
    button->SetBorderThickness(4.0);

    NeoWPF::Rendering::CommandList commands;
    button->BuildRenderCommands(commands);

    ASSERT_FALSE(commands.commands.empty());
    EXPECT_TRUE(ContainsRoundedRectColor(commands, 0xFF112233u));
    EXPECT_TRUE(ContainsRoundedRectColor(commands, 0xFF445566u));

    const auto background = std::dynamic_pointer_cast<NeoWPF::UI::Templates::TemplateVisual>(
        button->FindTemplateChild("PART_Background"));
    ASSERT_NE(background, nullptr);
    EXPECT_EQ(background->GetWidth(), 120.0);
    EXPECT_EQ(background->GetHeight(), 40.0);
}

TEST_F(ButtonTests, VisualStatesProduceDeterministicStateColors)
{
    auto button = MakeSizedButton();
    std::shared_ptr<NeoWPF::UI::Brush> hover;
    std::shared_ptr<NeoWPF::UI::Brush> accent;
    std::shared_ptr<NeoWPF::UI::Brush> pressed;
    auto& themeManager = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::AccentHover, hover));
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Accent, accent));
    ASSERT_TRUE(themeManager.TryGetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::AccentPressed, pressed));

    RaisePointerEvent(*button, NeoWPF::Input::Core::PointerEnteredEvent(),
                      NeoWPF::Input::Core::PointerPhase::Enter);
    NeoWPF::Rendering::CommandList mouseOverCommands;
    button->BuildRenderCommands(mouseOverCommands);
    EXPECT_TRUE(ContainsRoundedRectColor(mouseOverCommands, GetSolidBrushColor(hover)));
    EXPECT_TRUE(ContainsRoundedRectColor(mouseOverCommands, GetSolidBrushColor(accent)));

    RaisePointerEvent(*button, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    NeoWPF::Rendering::CommandList pressedCommands;
    button->BuildRenderCommands(pressedCommands);
    EXPECT_TRUE(ContainsRoundedRectColor(pressedCommands, GetSolidBrushColor(pressed)));
    EXPECT_TRUE(ContainsRoundedRectColor(pressedCommands, GetSolidBrushColor(accent)));

    RaisePointerEvent(*button, NeoWPF::Input::Core::PointerExitedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Leave);
    NeoWPF::Rendering::CommandList normalCommands;
    button->BuildRenderCommands(normalCommands);
    EXPECT_TRUE(ContainsRoundedRectColor(normalCommands, GetSolidBrushColor(button->GetBackgroundBrush())));
    EXPECT_TRUE(ContainsRoundedRectColor(normalCommands, GetSolidBrushColor(button->GetBorderBrush())));
}

TEST_F(ButtonTests, DisabledButtonDoesNotRaiseClickOrExecuteCommand)
{
    NeoWPF::UI::Button button;
    int clickCount = 0;
    button.Click += [&clickCount]() { ++clickCount; };
    auto command = std::make_shared<RecordingCommand>();
    button.SetCommand(command);
    button.SetCommandParameter(std::wstring(L"disabled"));
    button.SetEnabled(false);

    RaisePointerEvent(button, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(button, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_EQ(clickCount, 0);
    EXPECT_EQ(command->executeCount, 0);
}

TEST_F(ButtonTests, LeavingNestedContentCancelsPendingPointerClick)
{
    auto button = std::make_shared<NeoWPF::UI::Button>(L"Nested content");
    auto content = std::make_shared<NeoWPF::UI::Button>(L"Nested target");
    button->SetWidth(120.0);
    button->SetHeight(40.0);
    button->ApplyTemplate();
    button->Initialize();
    button->Measure(120.0, 40.0);
    button->Arrange(120.0, 40.0);
    button->AddVisualChild(content);
    content->Measure(60.0, 20.0);
    content->Arrange(60.0, 20.0);

    double contentX = 0.0;
    double contentY = 0.0;
    content->GetAbsolutePosition(contentX, contentY);
    const double insideX = contentX + content->GetFinalWidth() / 2.0;
    const double insideY = contentY + content->GetFinalHeight() / 2.0;
    int clickCount = 0;
    button->Click += [&clickCount] { ++clickCount; };

    auto& input = NeoWPF::Input::InputManager::GetInstance();
    input.NotifyVisualTreeReplaced();
    input.SetRootElement(button.get());

    const auto submit = [&input](NeoWPF::Input::Core::PointerPhase phase,
                                 double x, double y,
                                 std::uint32_t changedButton = 0,
                                 std::uint32_t buttons = 0) {
        const NeoWPF::Input::Core::PointerEventArgs args(
            1, NeoWPF::Input::Core::PointerDeviceType::Mouse, phase,
            x, y, buttons, changedButton);
        input.SubmitPointerEvent(args);
    };

    auto* hitTarget = input.HitTest(static_cast<int>(insideX), static_cast<int>(insideY));
    ASSERT_EQ(hitTarget, content.get());
    submit(NeoWPF::Input::Core::PointerPhase::Move, insideX, insideY);
    submit(NeoWPF::Input::Core::PointerPhase::Down, insideX, insideY, 0x01u, 0x01u);
    submit(NeoWPF::Input::Core::PointerPhase::Move, 150.0, insideY);
    submit(NeoWPF::Input::Core::PointerPhase::Move, insideX, insideY);
    submit(NeoWPF::Input::Core::PointerPhase::Up, insideX, insideY, 0x01u);

    EXPECT_EQ(clickCount, 0);
    input.NotifyVisualTreeReplaced();
}

TEST_F(ButtonTests, PointerClickRaisesClickAndExecutesCommandWithParameter)
{
    NeoWPF::UI::Button button;
    int clickCount = 0;
    button.Click += [&clickCount]() { ++clickCount; };
    auto command = std::make_shared<RecordingCommand>();
    button.SetCommand(command);
    button.SetCommandParameter(std::wstring(L"save"));

    RaisePointerEvent(button, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(button, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_EQ(clickCount, 1);
    EXPECT_EQ(command->executeCount, 1);
    ASSERT_TRUE(command->receivedParameter.has_value());
    EXPECT_EQ(std::any_cast<std::wstring>(command->receivedParameter), L"save");
}

TEST_F(ButtonTests, XamlPropertiesMatchEquivalentCppValues)
{
    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Button Text="Apply" Foreground="#FF102030" Background="#FF203040" BorderBrush="#FF304050" BorderThickness="3" Padding="1,2,3,4" FontSize="22" FontFamily="Test Sans" FontWeight="700" FontStyle="Italic" TextHorizontalAlignment="Right" TextVerticalAlignment="Bottom" />)");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);
    auto* button = dynamic_cast<NeoWPF::UI::Button*>(xamlTree.get());
    ASSERT_NE(button, nullptr);

    EXPECT_EQ(button->GetText(), L"Apply");
    EXPECT_EQ(GetSolidBrushColor(button->GetForeground()), 0xFF102030u);
    EXPECT_EQ(GetSolidBrushColor(button->GetBackgroundBrush()), 0xFF203040u);
    EXPECT_EQ(GetSolidBrushColor(button->GetBorderBrush()), 0xFF304050u);
    EXPECT_DOUBLE_EQ(button->GetBorderThickness(), 3.0);
    EXPECT_EQ(button->GetPadding(), NeoWPF::UI::Thickness(1, 2, 3, 4));
    EXPECT_EQ(button->GetFontSize(), 22);
    EXPECT_EQ(button->GetFontFamily(), L"Test Sans");
    EXPECT_EQ(button->GetFontWeight(), 700);
    EXPECT_EQ(button->GetFontStyle(), NeoWPF::UI::FontStyle::Italic);
    EXPECT_EQ(button->GetTextHorizontalAlignment(), NeoWPF::UI::TextHorizontalAlignment::Right);
    EXPECT_EQ(button->GetTextVerticalAlignment(), NeoWPF::UI::TextVerticalAlignment::Bottom);
}

} 
