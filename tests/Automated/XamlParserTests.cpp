#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Markup/Xaml/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/XamlMarkupExtension.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTreeBuilder.h>

#include <string>

TEST(XamlMarkupExtensionTests, ParsesRegisteredBindingExtension)
{
    NeoWPF::Core::Application::Initialize();

    std::string error;
    const auto extension = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
        "{Binding Name, Mode=TwoWay}",
        {1, 1},
        error);

    EXPECT_NE(extension, nullptr);
    EXPECT_TRUE(error.empty());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlMarkupExtensionTests, RejectsInvalidBindingArguments)
{
    NeoWPF::Core::Application::Initialize();

    for (const auto& value : {"{Binding}", "{Binding Name, Mode=Invalid}", "{Binding Name, Unknown=Value}"}) {
        std::string error;
        const auto extension = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(value, {1, 1}, error);
        EXPECT_EQ(extension, nullptr) << value;
        EXPECT_FALSE(error.empty()) << value;
    }

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, ParsesSimpleElementTree)
{
    const std::string xml = R"(
        <StackPanel>
            <Button Text="Hello" />
            <TextBlock Text="World" />
        </StackPanel>
    )";

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(xml);
    ASSERT_EQ(root.size(), 1u);
    EXPECT_EQ(root[0].Name, "StackPanel");
    ASSERT_EQ(root[0].Children.size(), 2u);
    EXPECT_EQ(root[0].Children[0].Name, "Button");
    EXPECT_EQ(root[0].Children[0].Attributes.at("Text"), "Hello");
    EXPECT_EQ(root[0].Children[1].Name, "TextBlock");
    EXPECT_EQ(root[0].Children[1].Attributes.at("Text"), "World");
}

TEST(XamlParserTests, DecodesXmlEntitiesInAttributesAndText)
{
    const std::string xml = R"(
        <TextBlock Text="Tom &amp; Jerry &lt;3">A &amp; B</TextBlock>
    )";

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(xml);
    ASSERT_EQ(root.size(), 1u);
    ASSERT_EQ(root[0].Name, "TextBlock");
    EXPECT_EQ(root[0].Attributes.at("Text"), "Tom & Jerry <3");
    ASSERT_EQ(root[0].Children.size(), 1u);
    EXPECT_EQ(root[0].Children[0].TextContent, "A & B");
}

TEST(XamlParserTests, BuildsLiveTreeFromXaml)
{
    NeoWPF::Core::Application::Initialize();

    const std::string xml = R"(
        <StackPanel>
            <TextBlock Text="Hello" />
            <Button Text="Click me" />
        </StackPanel>
    )";

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(xml);
    const auto liveTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);

    ASSERT_TRUE(liveTree != nullptr);

    const auto* panel = dynamic_cast<const NeoWPF::UI::StackPanel*>(liveTree.get());
    ASSERT_NE(panel, nullptr);

    const auto children = liveTree->GetLogicalChildren();
    ASSERT_EQ(children.size(), 2u);

    auto* firstText = dynamic_cast<NeoWPF::UI::TextBlock*>(children[0]);
    auto* secondButton = dynamic_cast<NeoWPF::UI::Button*>(children[1]);
    ASSERT_NE(firstText, nullptr);
    ASSERT_NE(secondButton, nullptr);
    EXPECT_EQ(firstText->GetText(), L"Hello");
    EXPECT_EQ(secondButton->GetText(), L"Click me");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, ParsesCommonScalarPropertiesViaPropertyMetadata)
{
    NeoWPF::Core::Application::Initialize();

    const std::string xml = R"(
        <StackPanel Width="120" Height="40" Background="#FF202020">
            <TextBlock Margin="10,5,10,5" HorizontalAlignment="Center" Text="Hello" />
        </StackPanel>
    )";

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(xml);
    const auto liveTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);

    ASSERT_TRUE(liveTree != nullptr);
    auto* panel = dynamic_cast<NeoWPF::UI::StackPanel*>(liveTree.get());
    ASSERT_NE(panel, nullptr);

    EXPECT_EQ(panel->GetWidth(), 120);
    EXPECT_EQ(panel->GetHeight(), 40);
    EXPECT_EQ(panel->GetBackgroundColor().ToArgb(), 0xFF202020u);

    const auto children = liveTree->GetLogicalChildren();
    ASSERT_EQ(children.size(), 1u);

    auto* text = dynamic_cast<NeoWPF::UI::TextBlock*>(children[0]);
    ASSERT_NE(text, nullptr);
    EXPECT_EQ(text->GetMargin(), NeoWPF::UI::Thickness(10, 5, 10, 5));
    EXPECT_EQ(text->GetHorizontalAlignment(), NeoWPF::UI::HorizontalAlignment::Center);
    EXPECT_EQ(text->GetText(), L"Hello");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, BuildsGridWithDefinitionsAndAttachedLayoutProperties)
{
    NeoWPF::Core::Application::Initialize();

    const std::string xml = R"(
        <Grid Background="#FF101418">
            <Grid.RowDefinitions>
                <RowDefinition Height="Auto" />
                <RowDefinition Height="*" />
            </Grid.RowDefinitions>
            <Grid.ColumnDefinitions>
                <ColumnDefinition Width="Auto" />
                <ColumnDefinition Width="*" />
                <ColumnDefinition Width="Auto" />
            </Grid.ColumnDefinitions>
            <Border Grid.Row="0" Grid.Column="0" Grid.ColumnSpan="3" Background="#FF243244" />
            <Checkbox Grid.Row="1" Grid.Column="0" Text="Enable" IsChecked="True" />
            <TextBox Grid.Row="1" Grid.Column="1" Text="search here" />
            <Button Grid.Row="1" Grid.Column="2" Text="Go" />
        </Grid>
    )";

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(xml);
    const auto liveTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);

    ASSERT_TRUE(liveTree != nullptr);

    auto* grid = dynamic_cast<NeoWPF::UI::Grid*>(liveTree.get());
    ASSERT_NE(grid, nullptr);

    EXPECT_EQ(grid->GetBackgroundColor().ToArgb(), 0xFF101418u);
    EXPECT_EQ(grid->GetVisualChildren().size(), 4u);

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, CollectionPropertiesProcessRowAndColumnDefinitionsAsCollection)
{
    NeoWPF::Core::Application::Initialize();

    const std::string xml = R"(
        <Grid>
            <Grid.RowDefinitions>
                <RowDefinition Height="20" />
                <RowDefinition Height="*" />
            </Grid.RowDefinitions>
            <Grid.ColumnDefinitions>
                <ColumnDefinition Width="30" />
                <ColumnDefinition Width="*" />
            </Grid.ColumnDefinitions>
        </Grid>
    )";

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(xml);
    const auto liveTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);

    ASSERT_TRUE(liveTree != nullptr);

    auto* grid = dynamic_cast<NeoWPF::UI::Grid*>(liveTree.get());
    ASSERT_NE(grid, nullptr);

    const auto visualChildren = grid->GetVisualChildren();
    EXPECT_EQ(visualChildren.size(), 0u) << "RowDefinitions and ColumnDefinitions should not be added as visual children";

    const auto logicalChildren = grid->GetLogicalChildren();
    EXPECT_EQ(logicalChildren.size(), 0u) << "Definition objects should not appear in logical children";

    NeoWPF::Core::Application::Shutdown();
}
