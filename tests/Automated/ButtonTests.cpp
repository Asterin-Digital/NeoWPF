#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Input/Command.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>

#include <any>
#include <memory>
#include <string>

namespace {

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
            command.roundedRect.color == color) {
            return true;
        }
    }
    return false;
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
    const auto& textColor = NeoWPF::UI::Button::TextColorProperty();
    const auto& fontSize = NeoWPF::UI::Button::FontSizeProperty();
    const auto& fontFamily = NeoWPF::UI::Button::FontFamilyProperty();
    const auto& fontWeight = NeoWPF::UI::Button::FontWeightProperty();
    const auto& fontStyle = NeoWPF::UI::Button::FontStyleProperty();
    const auto& horizontal = NeoWPF::UI::Button::TextHorizontalAlignmentProperty();
    const auto& vertical = NeoWPF::UI::Button::TextVerticalAlignmentProperty();

    EXPECT_EQ(&foreground, &textColor);
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
    const auto textColor = NeoWPF::UI::Color::FromArgb(0xFF112233u);

    button.SetText(L"Save");
    button.SetTextColor(textColor);
    button.SetFontSize(21);
    button.SetFontFamily(L"Test Sans");
    button.SetFontWeight(700);
    button.SetFontStyle(NeoWPF::UI::FontStyle::Italic);
    button.SetTextHorizontalAlignment(NeoWPF::UI::TextHorizontalAlignment::Right);
    button.SetTextVerticalAlignment(NeoWPF::UI::TextVerticalAlignment::Bottom);
    button.SetPadding(NeoWPF::UI::Thickness(1, 2, 3, 4));
    button.SetBackgroundColor(NeoWPF::UI::Color::FromArgb(0xFF445566u));
    button.SetBorderColor(NeoWPF::UI::Color::FromArgb(0xFF778899u));
    button.SetBorderThickness(3.0);

    EXPECT_EQ(button.GetValue<std::wstring>(NeoWPF::UI::Button::TextProperty()), L"Save");
    EXPECT_EQ(button.GetValue<NeoWPF::UI::Color>(NeoWPF::UI::Button::ForegroundProperty()), textColor);
    EXPECT_EQ(button.GetValue<int>(NeoWPF::UI::Button::FontSizeProperty()), 21);
    EXPECT_EQ(button.GetValue<std::wstring>(NeoWPF::UI::Button::FontFamilyProperty()), L"Test Sans");
    EXPECT_EQ(button.GetValue<int>(NeoWPF::UI::Button::FontWeightProperty()), 700);
    EXPECT_EQ(button.GetValue<NeoWPF::UI::FontStyle>(NeoWPF::UI::Button::FontStyleProperty()), NeoWPF::UI::FontStyle::Italic);
    EXPECT_EQ(button.GetValue<NeoWPF::UI::TextHorizontalAlignment>(NeoWPF::UI::Button::TextHorizontalAlignmentProperty()), NeoWPF::UI::TextHorizontalAlignment::Right);
    EXPECT_EQ(button.GetValue<NeoWPF::UI::TextVerticalAlignment>(NeoWPF::UI::Button::TextVerticalAlignmentProperty()), NeoWPF::UI::TextVerticalAlignment::Bottom);
    EXPECT_EQ(button.GetPadding(), NeoWPF::UI::Thickness(1, 2, 3, 4));
    EXPECT_EQ(button.GetBackgroundColor(), NeoWPF::UI::Color::FromArgb(0xFF445566u));
    EXPECT_EQ(button.GetBorderColor(), NeoWPF::UI::Color::FromArgb(0xFF778899u));
    EXPECT_DOUBLE_EQ(button.GetBorderThickness(), 3.0);

    EXPECT_EQ(button.GetText(), L"Save");
    EXPECT_EQ(button.GetTextColor(), textColor);
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
    const auto background = NeoWPF::UI::Color::FromArgb(0xFF102030u);
    const auto localBackground = NeoWPF::UI::Color::FromArgb(0xFF405060u);

    ASSERT_TRUE(button.SetStylePropertyValue("Background", background));
    ASSERT_TRUE(button.SetStylePropertyValue("BorderBrush", NeoWPF::UI::Color::FromArgb(0xFF708090u)));
    ASSERT_TRUE(button.SetStylePropertyValue("FontSize", 19));

    EXPECT_EQ(button.GetBackgroundColor(), background);
    EXPECT_EQ(button.GetBorderColor(), NeoWPF::UI::Color::FromArgb(0xFF708090u));
    EXPECT_EQ(button.GetFontSize(), 19);
    EXPECT_EQ(button.GetValueSource(NeoWPF::UI::Control::BackgroundProperty()), NeoWPF::UI::Properties::PropertyValueSource::Style);

    button.SetBackgroundColor(localBackground);
    EXPECT_EQ(button.GetBackgroundColor(), localBackground);
    EXPECT_EQ(button.GetValueSource(NeoWPF::UI::Control::BackgroundProperty()), NeoWPF::UI::Properties::PropertyValueSource::Local);

    button.ClearValue(NeoWPF::UI::Control::BackgroundProperty());
    EXPECT_EQ(button.GetBackgroundColor(), background);
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

    NeoWPF::UI::Color surface;
    NeoWPF::UI::Color foreground;
    NeoWPF::UI::Color border;
    ASSERT_TRUE(themeManager.TryGetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Surface, surface));
    ASSERT_TRUE(themeManager.TryGetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Foreground, foreground));
    ASSERT_TRUE(themeManager.TryGetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Border, border));

    auto lightButton = std::make_shared<NeoWPF::UI::Button>();
    EXPECT_EQ(lightButton->GetBackgroundColor(), surface);
    EXPECT_EQ(lightButton->GetTextColor(), foreground);
    EXPECT_EQ(lightButton->GetBorderColor(), border);

    ASSERT_TRUE(themeManager.SetActiveTheme("Dark Mint"));
    ASSERT_TRUE(themeManager.TryGetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Surface, surface));
    ASSERT_TRUE(themeManager.TryGetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Foreground, foreground));
    ASSERT_TRUE(themeManager.TryGetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Border, border));

    auto darkButton = std::make_shared<NeoWPF::UI::Button>();
    EXPECT_EQ(darkButton->GetBackgroundColor(), surface);
    EXPECT_EQ(darkButton->GetTextColor(), foreground);
    EXPECT_EQ(darkButton->GetBorderColor(), border);
}

TEST_F(ButtonTests, NormalStateUsesDependencyPropertyColorsAndHonorsBorderThickness)
{
    auto button = MakeSizedButton();
    button->SetBackgroundColor(NeoWPF::UI::Color::FromArgb(0xFF112233u));
    button->SetBorderColor(NeoWPF::UI::Color::FromArgb(0xFF445566u));
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
    NeoWPF::UI::Color hover;
    NeoWPF::UI::Color accent;
    NeoWPF::UI::Color pressed;
    auto& themeManager = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
    ASSERT_TRUE(themeManager.TryGetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::AccentHover, hover));
    ASSERT_TRUE(themeManager.TryGetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Accent, accent));
    ASSERT_TRUE(themeManager.TryGetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::AccentPressed, pressed));

    button->OnMouseEnter(nullptr);
    NeoWPF::Rendering::CommandList mouseOverCommands;
    button->BuildRenderCommands(mouseOverCommands);
    EXPECT_TRUE(ContainsRoundedRectColor(mouseOverCommands, hover.ToArgb()));
    EXPECT_TRUE(ContainsRoundedRectColor(mouseOverCommands, accent.ToArgb()));

    button->OnMouseDown(10, 10, 0x01u);
    NeoWPF::Rendering::CommandList pressedCommands;
    button->BuildRenderCommands(pressedCommands);
    EXPECT_TRUE(ContainsRoundedRectColor(pressedCommands, pressed.ToArgb()));
    EXPECT_TRUE(ContainsRoundedRectColor(pressedCommands, accent.ToArgb()));

    button->OnMouseLeave(nullptr);
    NeoWPF::Rendering::CommandList normalCommands;
    button->BuildRenderCommands(normalCommands);
    EXPECT_TRUE(ContainsRoundedRectColor(normalCommands, button->GetBackgroundColor().ToArgb()));
    EXPECT_TRUE(ContainsRoundedRectColor(normalCommands, button->GetBorderColor().ToArgb()));
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

    button.OnMouseDown(0, 0, 0x01u);
    button.OnMouseUp(0, 0, 0u);

    EXPECT_EQ(clickCount, 0);
    EXPECT_EQ(command->executeCount, 0);
}

TEST_F(ButtonTests, MouseClickRaisesClickAndExecutesCommandWithParameter)
{
    NeoWPF::UI::Button button;
    int clickCount = 0;
    button.Click += [&clickCount]() { ++clickCount; };
    auto command = std::make_shared<RecordingCommand>();
    button.SetCommand(command);
    button.SetCommandParameter(std::wstring(L"save"));

    button.OnMouseDown(0, 0, 0x01u);
    button.OnMouseUp(0, 0, 0u);

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
    EXPECT_EQ(button->GetTextColor(), NeoWPF::UI::Color::FromArgb(0xFF102030u));
    EXPECT_EQ(button->GetBackgroundColor(), NeoWPF::UI::Color::FromArgb(0xFF203040u));
    EXPECT_EQ(button->GetBorderColor(), NeoWPF::UI::Color::FromArgb(0xFF304050u));
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
