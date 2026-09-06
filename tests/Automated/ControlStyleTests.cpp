#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>

namespace NeoWPF::UI {
void ApplyTextBlockThemeStyle(UIElement* element);
}

namespace {

class ControlStyleTests : public testing::Test {
protected:
    void SetUp() override
    {
        NeoWPF::Core::Application::Initialize();
        auto& themes = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
        themes.RegisterBuiltInThemes();
        ASSERT_TRUE(themes.SetActiveTheme("Light"));
    }

    void TearDown() override
    {
        NeoWPF::Core::Application::Shutdown();
    }
};

}

TEST_F(ControlStyleTests, LightThemeUsesBlackForegroundForControls)
{
    NeoWPF::UI::Button button;
    NeoWPF::UI::Checkbox checkbox;
    NeoWPF::UI::TextBlock textBlock;
    NeoWPF::UI::TextBox textBox;

    EXPECT_EQ(button.GetTextColor().ToArgb(), 0xFF000000u);
    EXPECT_EQ(checkbox.GetTextColor().ToArgb(), 0xFF000000u);
    EXPECT_EQ(textBlock.GetTextColor().ToArgb(), 0xFF000000u);
    EXPECT_EQ(textBox.GetTextColor().ToArgb(), 0xFF000000u);
}

TEST_F(ControlStyleTests, TextBlockThemeStyleSetsRegisteredForegroundProperty)
{
    NeoWPF::UI::TextBlock textBlock;

    NeoWPF::UI::ApplyTextBlockThemeStyle(&textBlock);

    const auto* foreground = textBlock.FindDependencyProperty("Foreground");
    ASSERT_NE(foreground, nullptr);
    EXPECT_EQ(textBlock.GetValueSource(*foreground),
              NeoWPF::UI::Properties::PropertyValueSource::Style);
    EXPECT_EQ(textBlock.GetTextColor().ToArgb(), 0xFF000000u);
}

TEST_F(ControlStyleTests, CheckboxThemeStyleSetsBlackForeground)
{
    NeoWPF::UI::Checkbox checkbox;

    NeoWPF::UI::ApplyCheckboxThemeStyle(&checkbox);

    EXPECT_EQ(checkbox.GetTextColor().ToArgb(), 0xFF000000u);
    const auto* foreground = checkbox.FindDependencyProperty("Foreground");
    ASSERT_NE(foreground, nullptr);
    EXPECT_EQ(checkbox.GetValueSource(*foreground),
              NeoWPF::UI::Properties::PropertyValueSource::Style);
}

TEST_F(ControlStyleTests, XamlForegroundPreservesExplicitArgbColor)
{
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<TextBlock Text="Label" Foreground="#FF102030" />)");
    const auto textBlock = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);

    ASSERT_NE(textBlock, nullptr);
    const auto* label = dynamic_cast<const NeoWPF::UI::TextBlock*>(textBlock.get());
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(label->GetTextColor().ToArgb(), 0xFF102030u);
}

TEST_F(ControlStyleTests, ExplicitStyleAppliesTypedSetterDuringInitialization)
{
    auto style = std::make_shared<NeoWPF::UI::Styling::Style>();
    style->SetTargetType(std::type_index(typeid(NeoWPF::UI::TextBlock)));
    style->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Foreground", "#FF102030"));

    ASSERT_EQ(style->GetTargetType(), std::type_index(typeid(NeoWPF::UI::TextBlock)));
    ASSERT_EQ(style->GetSetters().size(), 1u);

    NeoWPF::UI::TextBlock textBlock;
    textBlock.SetValue(NeoWPF::UI::UIElement::StyleProperty(), style);
    ASSERT_EQ(textBlock.GetValue<std::shared_ptr<NeoWPF::UI::Styling::Style>>(
                  NeoWPF::UI::UIElement::StyleProperty()), style);
    textBlock.Initialize();

    const auto* foreground = textBlock.FindDependencyProperty("Foreground");
    ASSERT_NE(foreground, nullptr);
    EXPECT_EQ(textBlock.GetValueSource(*foreground),
              NeoWPF::UI::Properties::PropertyValueSource::Style);
    EXPECT_EQ(textBlock.GetTextColor().ToArgb(), 0xFF102030u);
}

TEST_F(ControlStyleTests, ImplicitStyleAppliesFromParentResourceScope)
{
    auto style = std::make_shared<NeoWPF::UI::Styling::Style>();
    style->SetTargetType(std::type_index(typeid(NeoWPF::UI::Button)));
    style->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Text", std::wstring(L"Implicit")));

    NeoWPF::UI::StackPanel panel;
    ASSERT_TRUE(panel.SetImplicitXamlStyle(typeid(NeoWPF::UI::Button), style));

    auto button = std::make_shared<NeoWPF::UI::Button>();
    panel.AddChild(button);
    panel.Initialize();

    EXPECT_EQ(button->GetText(), L"Implicit");
    EXPECT_EQ(button->GetValueSource(NeoWPF::UI::Button::TextProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Style);
}
