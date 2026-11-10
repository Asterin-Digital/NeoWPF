#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Data/INotifyPropertyChanged.h>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Localization/LocaleTable.h>
#include <NeoWPF/UI/Localization/LocalizationManager.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlNamescope.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/IXamlResourceHost.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>

#include <any>
#include <string>

namespace {

std::shared_ptr<NeoWPF::UI::Brush> MakeSolidColorBrush(std::uint32_t argb)
{
    return std::make_shared<NeoWPF::UI::SolidColorBrush>(
        argb);
}

void ExpectSolidColorBrush(const std::shared_ptr<NeoWPF::UI::Brush>& brush,
                           std::uint32_t argb)
{
    ASSERT_NE(brush, nullptr);
    const auto solidBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(brush);
    ASSERT_NE(solidBrush, nullptr);
    EXPECT_EQ(solidBrush->GetArgb(), argb);
}

void ExpectLayoutParity(const NeoWPF::UI::Visual& xamlRoot,
                        const NeoWPF::UI::Visual& cppRoot)
{
    const auto xamlDescendants = xamlRoot.GetVisualDescendants();
    const auto cppDescendants = cppRoot.GetVisualDescendants();
    ASSERT_EQ(xamlDescendants.size(), cppDescendants.size());

    for (std::size_t index = 0; index < xamlDescendants.size(); ++index) {
        const auto* const xamlDescendant = xamlDescendants[index];
        const auto* const cppDescendant = cppDescendants[index];
        ASSERT_NE(xamlDescendant, nullptr);
        ASSERT_NE(cppDescendant, nullptr);
        EXPECT_EQ(typeid(*xamlDescendant), typeid(*cppDescendant));

        const auto* xamlElement = dynamic_cast<const NeoWPF::UI::FrameworkElement*>(xamlDescendant);
        const auto* cppElement = dynamic_cast<const NeoWPF::UI::FrameworkElement*>(cppDescendant);
        ASSERT_EQ(xamlElement != nullptr, cppElement != nullptr);
        if (!xamlElement || !cppElement) {
            continue;
        }

        EXPECT_EQ(xamlElement->GetDesiredWidth(), cppElement->GetDesiredWidth());
        EXPECT_EQ(xamlElement->GetDesiredHeight(), cppElement->GetDesiredHeight());
        EXPECT_EQ(xamlElement->GetFinalWidth(), cppElement->GetFinalWidth());
        EXPECT_EQ(xamlElement->GetFinalHeight(), cppElement->GetFinalHeight());
        EXPECT_EQ(xamlElement->GetLayoutOffsetX(), cppElement->GetLayoutOffsetX());
        EXPECT_EQ(xamlElement->GetLayoutOffsetY(), cppElement->GetLayoutOffsetY());
    }
}

std::shared_ptr<NeoWPF::UI::Visual> AsVisual(
    const std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>& root)
{
    return std::dynamic_pointer_cast<NeoWPF::UI::Visual>(root);
}

}

TEST(XamlRegistrationTests, BuiltInRegistrationsValidate)
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::ClearDeveloperWarnings();

    EXPECT_TRUE(NeoWPF::UI::Markup::Xaml::ValidateXamlRegistrations());
    EXPECT_FALSE(NeoWPF::Diagnostics::GetLastWarning().has_value());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlRegistrationTests, BuiltInTypesExposeCodeGenerationMetadata)
{
    NeoWPF::Core::Application::Initialize();

    const auto& registry = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get();
    const auto registeredTypes = registry.GetRegisteredTypes();
    ASSERT_FALSE(registeredTypes.empty());
    for (const auto& registeredType : registeredTypes) {
        EXPECT_FALSE(registeredType.Info.CppTypeName.empty()) << registeredType.TagName;
        EXPECT_FALSE(registeredType.Info.HeaderPath.empty()) << registeredType.TagName;
    }

    EXPECT_EQ(registry.Find("Button")->CppTypeName, "NeoWPF::UI::Button");
    EXPECT_EQ(registry.Find("Button")->HeaderPath, "NeoWPF/UI/Controls/Button/Button.h");
    EXPECT_EQ(registry.Find("Grid")->CppTypeName, "NeoWPF::UI::Grid");
    EXPECT_EQ(registry.Find("Grid")->HeaderPath, "NeoWPF/UI/Layout/Grid/Grid.h");
    ASSERT_NE(registry.Find("ContentControl"), nullptr);
    EXPECT_EQ(registry.Find("ContentControl")->CppTypeName,
              "NeoWPF::UI::Content::ContentControl");
    EXPECT_EQ(registry.Find("ContentControl")->HeaderPath,
              "NeoWPF/UI/Content/ContentControl.h");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, ParsesTextContentOnListBoxItem)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <ListBox>
            <ListBoxItem Content="Alpha" />
            <ListBoxItem Content="Bravo" />
        </ListBox>
    )");
    ASSERT_EQ(root.size(), 1u);

    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(tree, nullptr);

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlMarkupExtensionTests, ParsesRegisteredBindingExtension)
{
    NeoWPF::Core::Application::Initialize();

    const auto extensionResult = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
        "{Binding Name, Mode=TwoWay}",
        {1, 1});

    ASSERT_TRUE(extensionResult.Success());
    EXPECT_NE(extensionResult.Value(), nullptr);

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlMarkupExtensionTests, RejectsInvalidBindingArguments)
{
    NeoWPF::Core::Application::Initialize();

    for (const auto& value : {"{Binding}", "{Binding Name, Mode=Invalid}", "{Binding Name, Unknown=Value}"}) {
        const auto extensionResult = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(value, {1, 1});
        ASSERT_TRUE(extensionResult.Fail()) << value;
        EXPECT_FALSE(extensionResult.Error().empty()) << value;
    }

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlMarkupExtensionTests, ParsesLocaleExtension)
{
    NeoWPF::Core::Application::Initialize();

    for (const auto& value : {"{Locale Greeting}", "{Locale Key=Greeting}"}) {
        const auto extensionResult = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
            value, {1, 1});

        ASSERT_TRUE(extensionResult.Success()) << value << ": " << extensionResult.Error();
        const auto& extension = extensionResult.Value();
        ASSERT_NE(extension, nullptr) << value;
        std::string key;
        ASSERT_TRUE(NeoWPF::UI::Markup::Xaml::TryGetXamlLocaleKey(*extension, key));
        EXPECT_EQ(key, "Greeting");
    }

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlMarkupExtensionTests, RejectsInvalidLocaleArguments)
{
    NeoWPF::Core::Application::Initialize();

    for (const auto& value : {
             "{Locale}",
             "{Locale Key=}",
             "{Locale Key=Greeting, Key=Other}",
             "{Locale Greeting, Other}",
             "{Locale Unknown=Greeting}"}) {
        const auto extensionResult = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
            value, {1, 1});

        ASSERT_TRUE(extensionResult.Fail()) << value;
        EXPECT_FALSE(extensionResult.Error().empty()) << value;
    }

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlRuntimeTests, LocaleExtensionUpdatesTextBlock)
{
    NeoWPF::Core::Application::Initialize();
    auto& manager = NeoWPF::UI::Localization::LocalizationManager::Get();
    manager.Clear();

    NeoWPF::UI::Localization::LocaleTable english(L"en-US");
    NeoWPF::UI::Localization::LocaleTable romanian(L"ro-RO");
    ASSERT_TRUE(english.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(romanian.Add(L"Greeting", L"Salut"));
    ASSERT_TRUE(manager.RegisterTable(english));
    ASSERT_TRUE(manager.RegisterTable(romanian));
    ASSERT_TRUE(manager.SetActiveLocale(L"en-US"));

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<TextBlock Text="{Locale Greeting}" />)");
    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    const auto textBlock = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(object);

    ASSERT_NE(textBlock, nullptr);
    EXPECT_EQ(textBlock->GetText(), L"Hello");
    ASSERT_TRUE(manager.SetActiveLocale(L"ro-RO"));
    EXPECT_EQ(textBlock->GetText(), L"Salut");

    manager.Clear();
    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlRuntimeTests, LocaleExtensionSupportsNamedKey)
{
    NeoWPF::Core::Application::Initialize();
    auto& manager = NeoWPF::UI::Localization::LocalizationManager::Get();
    manager.Clear();

    NeoWPF::UI::Localization::LocaleTable english(L"en-US");
    ASSERT_TRUE(english.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(manager.RegisterTable(english));
    ASSERT_TRUE(manager.SetActiveLocale(L"en-US"));

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<TextBlock Text="{Locale Key=Greeting}" />)");
    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    const auto textBlock = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(object);

    ASSERT_NE(textBlock, nullptr);
    EXPECT_EQ(textBlock->GetText(), L"Hello");
    EXPECT_NE(NeoWPF::UI::Localization::LocaleOperations::GetLocaleExpression(
                  *textBlock,
                  NeoWPF::UI::TextBlock::TextProperty()), nullptr);

    manager.Clear();
    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlRuntimeTests, LocaleExtensionUpdatesTextBoxAndDirectTextWins)
{
    NeoWPF::Core::Application::Initialize();
    auto& manager = NeoWPF::UI::Localization::LocalizationManager::Get();
    manager.Clear();

    NeoWPF::UI::Localization::LocaleTable english(L"en-US");
    NeoWPF::UI::Localization::LocaleTable romanian(L"ro-RO");
    ASSERT_TRUE(english.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(romanian.Add(L"Greeting", L"Salut"));
    ASSERT_TRUE(manager.RegisterTable(english));
    ASSERT_TRUE(manager.RegisterTable(romanian));
    ASSERT_TRUE(manager.SetActiveLocale(L"en-US"));

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<TextBox Text="{Locale Key=Greeting}" />)");
    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    const auto textBox = std::dynamic_pointer_cast<NeoWPF::UI::TextBox>(object);

    ASSERT_NE(textBox, nullptr);
    EXPECT_EQ(textBox->GetText(), L"Hello");
    ASSERT_TRUE(manager.SetActiveLocale(L"ro-RO"));
    EXPECT_EQ(textBox->GetText(), L"Salut");

    textBox->SetText(L"Custom");
    ASSERT_TRUE(manager.SetActiveLocale(L"en-US"));
    EXPECT_EQ(textBox->GetText(), L"Custom");
    EXPECT_EQ(NeoWPF::UI::Localization::LocaleOperations::GetLocaleExpression(
                  *textBox,
                  NeoWPF::UI::TextBox::TextProperty()), nullptr);

    manager.Clear();
    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlMarkupExtensionTests, RejectsEmptyStaticResourceKey)
{
    NeoWPF::Core::Application::Initialize();

    const auto extensionResult = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
        "{StaticResource}", {1, 1});

    ASSERT_TRUE(extensionResult.Fail());
    EXPECT_EQ(extensionResult.Error(), "StaticResource key is required");

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

TEST(XamlParserTests, BuildsStyleTargetTypeAndSetterCollection)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <Style TargetType="Button">
            <Setter Property="Text" Value="Styled" />
        </Style>
    )");
    ASSERT_EQ(root.size(), 1u);
    ASSERT_EQ(root[0].Attributes.at("TargetType"), "Button");
    ASSERT_NE(NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find("Button"), nullptr);
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);

    ASSERT_NE(tree, nullptr);
    const auto style = std::dynamic_pointer_cast<NeoWPF::UI::Styling::Style>(tree);
    ASSERT_NE(style, nullptr);
    EXPECT_EQ(style->GetTargetType(), std::type_index(typeid(NeoWPF::UI::Button)));
    ASSERT_EQ(style->GetSetters().size(), 1u);
    ASSERT_NE(style->GetSetters()[0], nullptr);
    EXPECT_EQ(style->GetSetters()[0]->GetProperty(), "Text");
    EXPECT_EQ(std::any_cast<std::string>(style->GetSetters()[0]->GetValue()), "Styled");

    NeoWPF::Core::Application::Shutdown();
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

TEST(XamlParserTests, PreservesXamlNameAsElementIdentity)
{
    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Grid x:Name="RootGrid"><Button x:Name="ApplyButton" Text="Apply" /></Grid>)");

    ASSERT_EQ(root.size(), 1u);
    EXPECT_EQ(root[0].XamlName, "RootGrid");
    EXPECT_EQ(root[0].Attributes.count("x:Name"), 0u);
    ASSERT_EQ(root[0].Children.size(), 1u);
    EXPECT_EQ(root[0].Children[0].XamlName, "ApplyButton");
    EXPECT_EQ(root[0].Children[0].Attributes.at("Text"), "Apply");
}

TEST(XamlParserTests, PreservesDocumentClassAsSpecialMetadata)
{
    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Application x:Class="SampleApp.App" />)");

    ASSERT_EQ(root.size(), 1u);
    EXPECT_EQ(root[0].XamlClass, "SampleApp.App");
    EXPECT_EQ(root[0].Attributes.count("x:Class"), 0u);
}

TEST(XamlParserTests, PreservesXamlKeyAsResourceIdentity)
{
    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<ResourceDictionary><TextBlock x:Key="Accent" Text="Hello" /></ResourceDictionary>)");

    ASSERT_EQ(root.size(), 1u);
    ASSERT_EQ(root[0].Children.size(), 1u);
    EXPECT_EQ(root[0].Children[0].XamlKey, "Accent");
    EXPECT_EQ(root[0].Children[0].Attributes.count("x:Key"), 0u);
}

TEST(XamlMarkupExtensionTests, ResolvesStaticResourceThroughContext)
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    auto button = std::make_shared<NeoWPF::UI::Button>();
    const auto extensionResult = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
        "{StaticResource Accent}",
        {1, 1, [](std::string_view key, std::any& value) {
            if (key != "Accent") {
                return false;
            }
            value = std::wstring(L"Resolved accent");
            return true;
        }});

    ASSERT_TRUE(extensionResult.Success()) << extensionResult.Error();
    const auto& extension = extensionResult.Value();
    ASSERT_NE(extension, nullptr);
    const auto applyResult = extension->Apply(
        *button,
        NeoWPF::UI::Button::TextProperty());
    ASSERT_TRUE(applyResult.Success()) << applyResult.Error();
    EXPECT_EQ(button->GetText(), L"Resolved accent");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, BuildsKeyedResourceDictionaryWithoutAddingVisualChildren)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <StackPanel>
            <StackPanel.Resources>
                <ResourceDictionary>
                    <TextBlock x:Key="Accent" Text="Resource text" />
                </ResourceDictionary>
            </StackPanel.Resources>
            <Button Text="Action" />
        </StackPanel>
    )");
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);

    ASSERT_NE(tree, nullptr);
    EXPECT_EQ(AsVisual(tree)->GetVisualChildren().size(), 1u);

    std::any resource;
    const auto* element = dynamic_cast<const NeoWPF::UI::UIElement*>(tree.get());
    ASSERT_NE(element, nullptr);
    ASSERT_TRUE(element->TryFindResource("Accent", resource));
    const auto resourceObject = std::any_cast<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>>(resource);
    ASSERT_NE(resourceObject, nullptr);
    const auto resourceText = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(resourceObject);
    ASSERT_NE(resourceText, nullptr);
    EXPECT_EQ(resourceText->GetText(), L"Resource text");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, StoresKeylessStyleByTargetType)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <StackPanel>
            <StackPanel.Resources>
                <ResourceDictionary>
                    <Style TargetType="Button">
                        <Setter Property="Text" Value="Implicit" />
                    </Style>
                </ResourceDictionary>
            </StackPanel.Resources>
        </StackPanel>
    )");
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);

    ASSERT_NE(tree, nullptr);
    const auto* resourceHost = dynamic_cast<const NeoWPF::UI::Markup::Xaml::IXamlResourceHost*>(tree.get());
    ASSERT_NE(resourceHost, nullptr);

    std::shared_ptr<NeoWPF::UI::Styling::Style> style;
    ASSERT_TRUE(resourceHost->TryFindImplicitStyle(typeid(NeoWPF::UI::Button), style));
    ASSERT_NE(style, nullptr);

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, ReportsDuplicateResourceKeys)
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::ClearDeveloperWarnings();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <StackPanel>
            <StackPanel.Resources>
                <ResourceDictionary>
                    <TextBlock x:Key="Accent" />
                    <TextBlock x:Key="Accent" />
                </ResourceDictionary>
            </StackPanel.Resources>
        </StackPanel>
    )");
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);

    ASSERT_NE(tree, nullptr);
    ASSERT_TRUE(NeoWPF::Diagnostics::GetLastWarning().has_value());
    EXPECT_EQ(NeoWPF::Diagnostics::GetLastWarning()->title, L"NEO-XAML107");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, ResolvesResourcesThroughNestedScopesAndShadowsOuterValues)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <StackPanel>
            <StackPanel.Resources>
                <ResourceDictionary>
                    <TextBlock x:Key="Accent" x:Name="OuterAccent" Text="Outer" />
                </ResourceDictionary>
            </StackPanel.Resources>
            <StackPanel>
                <StackPanel.Resources>
                    <ResourceDictionary>
                        <TextBlock x:Key="Accent" x:Name="InnerAccent" Text="Inner" />
                    </ResourceDictionary>
                </StackPanel.Resources>
                <Button Text="Inner" />
            </StackPanel>
            <Button Text="Outer" />
        </StackPanel>
    )");
    NeoWPF::UI::Markup::Xaml::XamlNamescope namescope;
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root, &namescope);

    ASSERT_NE(tree, nullptr);
    const auto children = AsVisual(tree)->GetVisualChildren();
    ASSERT_EQ(children.size(), 2u);

    auto* outer = dynamic_cast<NeoWPF::UI::UIElement*>(children[1]);
    auto* inner = dynamic_cast<NeoWPF::UI::UIElement*>(children[0]);
    ASSERT_NE(outer, nullptr);
    ASSERT_NE(inner, nullptr);

    std::any outerValue;
    std::any innerValue;
    ASSERT_TRUE(outer->TryFindResource("Accent", outerValue));
    ASSERT_TRUE(inner->TryFindResource("Accent", innerValue));
    const auto outerAccent = std::any_cast<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>>(outerValue);
    const auto innerAccent = std::any_cast<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>>(innerValue);
    ASSERT_NE(outerAccent, nullptr);
    ASSERT_NE(innerAccent, nullptr);
    EXPECT_NE(outerAccent.get(), innerAccent.get());
    EXPECT_EQ(std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(outerAccent)->GetText(), L"Outer");
    EXPECT_EQ(std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(innerAccent)->GetText(), L"Inner");
    EXPECT_EQ(namescope.Find<NeoWPF::UI::TextBlock>("OuterAccent")->GetText(), L"Outer");
    EXPECT_EQ(namescope.Find<NeoWPF::UI::TextBlock>("InnerAccent")->GetText(), L"Inner");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, FindsApplicationResourceAfterElementAndAncestorScopes)
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Core::Application::SetResource(
        "ApplicationAccent",
        std::make_shared<NeoWPF::UI::TextBlock>(L"Application"));

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<StackPanel><Button Text=\"Action\" /></StackPanel>");
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(tree, nullptr);

    std::any value;
    const auto* element = dynamic_cast<const NeoWPF::UI::UIElement*>(tree.get());
    ASSERT_NE(element, nullptr);
    ASSERT_TRUE(element->TryFindResource("ApplicationAccent", value));
    const auto resource = std::any_cast<std::shared_ptr<NeoWPF::UI::TextBlock>>(value);
    ASSERT_NE(resource, nullptr);
    EXPECT_EQ(resource->GetText(), L"Application");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, PreservesResourceObjectIdentityAcrossNameAndKey)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<StackPanel><StackPanel.Resources><ResourceDictionary><TextBlock x:Key="Accent" x:Name="AccentObject" /></ResourceDictionary></StackPanel.Resources></StackPanel>)");
    NeoWPF::UI::Markup::Xaml::XamlNamescope namescope;
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root, &namescope);

    ASSERT_NE(tree, nullptr);
    std::any value;
    const auto* element = dynamic_cast<const NeoWPF::UI::UIElement*>(tree.get());
    ASSERT_NE(element, nullptr);
    ASSERT_TRUE(element->TryFindResource("Accent", value));
    const auto keyed = std::any_cast<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>>(value);
    const auto named = namescope.Find<NeoWPF::UI::TextBlock>("AccentObject");
    ASSERT_NE(keyed, nullptr);
    ASSERT_NE(named, nullptr);
    EXPECT_EQ(keyed.get(), named.get());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlRootTests, BuildsWindowWithOwnedTitleAndComposedContent)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <Window Title="Temporary title">
            <Window.TitleBar>
                <TextBlock Text="Title bar" />
            </Window.TitleBar>
            <Window.Content>
                <Grid>
                    <TextBlock Text="Content" />
                </Grid>
            </Window.Content>
        </Window>
    )");
    const auto built = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);

    const auto window = std::dynamic_pointer_cast<NeoWPF::UI::Window>(built);
    ASSERT_NE(window, nullptr);
    EXPECT_STREQ(window->GetTitle(), L"Temporary title");
    ASSERT_NE(window->GetTitleBar(), nullptr);
    ASSERT_NE(window->GetContent(), nullptr);
    EXPECT_NE(std::dynamic_pointer_cast<NeoWPF::UI::Grid>(window->GetContent()), nullptr);

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlRootTests, BuildsApplicationResourcesAndResolvesThemFromWindow)
{
    NeoWPF::Core::Application::Initialize();

    const auto applicationRoot = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <Application>
            <Application.Resources>
                <ResourceDictionary>
                    <TextBlock x:Key="Accent" Text="Application accent" />
                </ResourceDictionary>
            </Application.Resources>
        </Application>
    )");
    const auto applicationObject = NeoWPF::UI::Markup::Xaml::BuildXamlTree(applicationRoot);
    ASSERT_NE(std::dynamic_pointer_cast<NeoWPF::Core::Application>(applicationObject), nullptr);

    std::any resource;
    ASSERT_TRUE(NeoWPF::Core::Application::TryFindResource("Accent", resource));
    const auto accent = std::any_cast<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>>(resource);
    ASSERT_NE(accent, nullptr);
    EXPECT_EQ(std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(accent)->GetText(), L"Application accent");

    const auto windowRoot = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <Window>
            <Window.Content>
                <ContentControl Content="{StaticResource Accent}" />
            </Window.Content>
        </Window>
    )");
    const auto windowObject = NeoWPF::UI::Markup::Xaml::BuildXamlTree(windowRoot);
    const auto window = std::dynamic_pointer_cast<NeoWPF::UI::Window>(windowObject);
    ASSERT_NE(window, nullptr);
    const auto composedContent = window->GetContent();
    ASSERT_NE(composedContent, nullptr);
    ASSERT_EQ(composedContent->GetVisualChildren().size(), 1u);
    const auto contentControl = dynamic_cast<NeoWPF::UI::Content::ContentControl*>(
        composedContent->GetVisualChildren().front());
    ASSERT_NE(contentControl, nullptr);
    EXPECT_EQ(contentControl->GetContentElement().get(), accent.get());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlRootTests, RegistersNamesInsideWindowRoot)
{
    NeoWPF::Core::Application::Initialize();

    NeoWPF::UI::Markup::Xaml::XamlNamescope namescope;
    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Window><Window.Content><TextBlock x:Name="NamedContent" Text="Named" /></Window.Content></Window>)");
    const auto built = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root, &namescope);

    ASSERT_NE(std::dynamic_pointer_cast<NeoWPF::UI::Window>(built), nullptr);
    const auto named = namescope.Find<NeoWPF::UI::TextBlock>("NamedContent");
    ASSERT_NE(named, nullptr);
    EXPECT_EQ(named->GetText(), L"Named");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlRootTests, ReportsApplicationResourceKeyFailures)
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::ClearDeveloperWarnings();

    const auto missingKeyRoot = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Application><Application.Resources><ResourceDictionary><TextBlock /></ResourceDictionary></Application.Resources></Application>)");
    ASSERT_NE(NeoWPF::UI::Markup::Xaml::BuildXamlTree(missingKeyRoot), nullptr);
    ASSERT_TRUE(NeoWPF::Diagnostics::GetLastWarning().has_value());
    EXPECT_EQ(NeoWPF::Diagnostics::GetLastWarning()->title, L"NEO-XAML107");

    NeoWPF::Diagnostics::ClearDeveloperWarnings();
    const auto duplicateKeyRoot = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <Application><Application.Resources><ResourceDictionary>
            <TextBlock x:Key="Accent" />
            <TextBlock x:Key="Accent" />
        </ResourceDictionary></Application.Resources></Application>
    )");
    ASSERT_NE(NeoWPF::UI::Markup::Xaml::BuildXamlTree(duplicateKeyRoot), nullptr);
    ASSERT_TRUE(NeoWPF::Diagnostics::GetLastWarning().has_value());
    EXPECT_EQ(NeoWPF::Diagnostics::GetLastWarning()->title, L"NEO-XAML107");

    NeoWPF::Core::Application::Shutdown();
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

    const auto children = AsVisual(liveTree)->GetLogicalChildren();
    ASSERT_EQ(children.size(), 2u);

    auto* firstText = dynamic_cast<NeoWPF::UI::TextBlock*>(children[0]);
    auto* secondButton = dynamic_cast<NeoWPF::UI::Button*>(children[1]);
    ASSERT_NE(firstText, nullptr);
    ASSERT_NE(secondButton, nullptr);
    EXPECT_EQ(firstText->GetText(), L"Hello");
    EXPECT_EQ(secondButton->GetText(), L"Click me");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, RegistersNamedElementsInNamescope)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<StackPanel x:Name="RootPanel"><Button x:Name="ApplyButton" Text="Apply" /></StackPanel>)");
    NeoWPF::UI::Markup::Xaml::XamlNamescope namescope;
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root, &namescope);

    ASSERT_NE(tree, nullptr);
    EXPECT_EQ(namescope.Find<NeoWPF::UI::StackPanel>("RootPanel").get(), tree.get());
    const auto button = namescope.Find<NeoWPF::UI::Button>("ApplyButton");
    ASSERT_NE(button, nullptr);
    EXPECT_EQ(button->GetText(), L"Apply");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, ReportsDuplicateNamesInNamescope)
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::ClearDeveloperWarnings();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<StackPanel><Button x:Name="Action" /><TextBlock x:Name="Action" /></StackPanel>)");
    NeoWPF::UI::Markup::Xaml::XamlNamescope namescope;
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root, &namescope);

    ASSERT_NE(tree, nullptr);
    const auto action = namescope.Find<NeoWPF::UI::Button>("Action");
    ASSERT_NE(action, nullptr);
    ASSERT_TRUE(NeoWPF::Diagnostics::GetLastWarning().has_value());
    EXPECT_EQ(NeoWPF::Diagnostics::GetLastWarning()->title, L"NEO-XAML104");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, AppliesPocoBindingValuesToXamlTargets)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<StackPanel><TextBlock Text="{Binding Message}" /><TextBox Text="{Binding Query, Mode=TwoWay}" PlaceholderText="Type a message" /></StackPanel>)");
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(tree, nullptr);

    auto viewModel = std::make_shared<NeoWPF::UI::Data::PropertyBagNotify>();
    viewModel->SetProperty("Message", L"Bound message");
    viewModel->SetProperty("Query", L"Bound query");
    auto* rootElement = dynamic_cast<NeoWPF::UI::FrameworkElement*>(tree.get());
    ASSERT_NE(rootElement, nullptr);
    rootElement->SetDataContext(viewModel);

    const auto children = AsVisual(tree)->GetVisualChildren();
    ASSERT_EQ(children.size(), 2u);
    ASSERT_EQ(dynamic_cast<NeoWPF::UI::TextBlock*>(children[0])->GetText(), L"Bound message");
    ASSERT_EQ(dynamic_cast<NeoWPF::UI::TextBox*>(children[1])->GetText(), L"Bound query");
    ASSERT_EQ(dynamic_cast<NeoWPF::UI::TextBox*>(children[1])->GetPlaceholderText(), L"Type a message");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, RefreshesBindingsWhenBorderContentIsMaterialized)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Border><StackPanel><TextBlock Text="{Binding PreviewTitle}" /><TextBlock Text="{Binding PreviewDetail}" /></StackPanel></Border>)");
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(tree, nullptr);

    auto viewModel = std::make_shared<NeoWPF::UI::Data::PropertyBagNotify>();
    viewModel->SetProperty("PreviewTitle", L"Preview is active");
    viewModel->SetProperty("PreviewDetail", L"The binding reached deferred content.");
    auto* rootElement = dynamic_cast<NeoWPF::UI::FrameworkElement*>(tree.get());
    ASSERT_NE(rootElement, nullptr);
    rootElement->SetDataContext(viewModel);

    rootElement->Measure(400, 200);
    rootElement->Arrange(400, 200);

    auto* border = dynamic_cast<NeoWPF::UI::Border*>(tree.get());
    ASSERT_NE(border, nullptr);
    auto* panel = dynamic_cast<NeoWPF::UI::StackPanel*>(border->GetChild().get());
    ASSERT_NE(panel, nullptr);
    const auto children = panel->GetVisualChildren();
    ASSERT_EQ(children.size(), 2u);
    EXPECT_EQ(dynamic_cast<NeoWPF::UI::TextBlock*>(children[0])->GetText(), L"Preview is active");
    EXPECT_EQ(dynamic_cast<NeoWPF::UI::TextBlock*>(children[1])->GetText(), L"The binding reached deferred content.");

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppConstructionProduceEquivalentTreeState)
{
    NeoWPF::Core::Application::Initialize();

    const std::string xml = R"(
        <StackPanel Width="320" Height="120">
            <Button Text="Action" />
            <TextBlock Text="Status" />
        </StackPanel>
    )";

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(xml);
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);

    auto cppTree = std::make_shared<NeoWPF::UI::StackPanel>();
    cppTree->SetWidth(320);
    cppTree->SetHeight(120);
    auto cppButton = std::make_shared<NeoWPF::UI::Button>();
    cppButton->SetText(L"Action");
    auto cppText = std::make_shared<NeoWPF::UI::TextBlock>();
    cppText->SetText(L"Status");
    cppTree->AddChild(cppButton);
    cppTree->AddChild(cppText);

    const auto* const xamlTreeVisual = xamlTree.get();
    const auto* const cppTreeVisual = cppTree.get();
    EXPECT_EQ(typeid(*xamlTreeVisual), typeid(*cppTreeVisual));
    EXPECT_EQ(AsVisual(xamlTree)->GetVisualChildren().size(), cppTree->GetVisualChildren().size());
    EXPECT_EQ(AsVisual(xamlTree)->GetLogicalChildren().size(), cppTree->GetLogicalChildren().size());
    EXPECT_EQ(AsVisual(xamlTree)->GetLogicalChildren().size(), 2u);

    const auto xamlChildren = AsVisual(xamlTree)->GetLogicalChildren();
    const auto cppChildren = cppTree->GetLogicalChildren();
    ASSERT_EQ(xamlChildren.size(), 2u);
    ASSERT_EQ(cppChildren.size(), 2u);
    const auto* const xamlFirstChild = xamlChildren[0];
    const auto* const cppFirstChild = cppChildren[0];
    const auto* const xamlSecondChild = xamlChildren[1];
    const auto* const cppSecondChild = cppChildren[1];
    EXPECT_EQ(typeid(*xamlFirstChild), typeid(*cppFirstChild));
    EXPECT_EQ(typeid(*xamlSecondChild), typeid(*cppSecondChild));

    const auto* xamlButton = dynamic_cast<const NeoWPF::UI::Button*>(xamlChildren[0]);
    const auto* cppButtonValue = dynamic_cast<const NeoWPF::UI::Button*>(cppChildren[0]);
    const auto* xamlText = dynamic_cast<const NeoWPF::UI::TextBlock*>(xamlChildren[1]);
    const auto* cppTextValue = dynamic_cast<const NeoWPF::UI::TextBlock*>(cppChildren[1]);
    ASSERT_NE(xamlButton, nullptr);
    ASSERT_NE(cppButtonValue, nullptr);
    ASSERT_NE(xamlText, nullptr);
    ASSERT_NE(cppTextValue, nullptr);

    EXPECT_EQ(xamlButton->GetText(), cppButtonValue->GetText());
    EXPECT_EQ(xamlText->GetText(), cppTextValue->GetText());
    const auto* xamlFrameworkElement = dynamic_cast<const NeoWPF::UI::FrameworkElement*>(xamlTree.get());
    const auto* cppFrameworkElement = dynamic_cast<const NeoWPF::UI::FrameworkElement*>(cppTree.get());
    ASSERT_NE(xamlFrameworkElement, nullptr);
    ASSERT_NE(cppFrameworkElement, nullptr);
    EXPECT_EQ(std::any_cast<double>(xamlFrameworkElement->GetRawValue(NeoWPF::UI::FrameworkElement::WidthProperty())),
              std::any_cast<double>(cppFrameworkElement->GetRawValue(NeoWPF::UI::FrameworkElement::WidthProperty())));
    EXPECT_EQ(std::any_cast<double>(xamlFrameworkElement->GetRawValue(NeoWPF::UI::FrameworkElement::HeightProperty())),
              std::any_cast<double>(cppFrameworkElement->GetRawValue(NeoWPF::UI::FrameworkElement::HeightProperty())));
    EXPECT_EQ(std::any_cast<std::wstring>(xamlButton->GetRawValue(NeoWPF::UI::Button::TextProperty())),
              std::any_cast<std::wstring>(cppButtonValue->GetRawValue(NeoWPF::UI::Button::TextProperty())));
    EXPECT_EQ(std::any_cast<std::wstring>(xamlText->GetRawValue(NeoWPF::UI::TextBlock::TextProperty())),
              std::any_cast<std::wstring>(cppTextValue->GetRawValue(NeoWPF::UI::TextBlock::TextProperty())));

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
    ExpectSolidColorBrush(panel->GetBackgroundBrush(), 0xFF202020u);

    const auto children = AsVisual(liveTree)->GetLogicalChildren();
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

    ExpectSolidColorBrush(grid->GetBackgroundBrush(), 0xFF101418u);
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

TEST(XamlParserTests, XamlAndCppMatchScalarLayoutProperties)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<StackPanel Width="240" Height="80" Margin="1,2,3,4" Spacing="6" />)");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);

    auto cppTree = std::make_shared<NeoWPF::UI::StackPanel>();
    cppTree->SetWidth(240);
    cppTree->SetHeight(80);
    cppTree->SetMargin(NeoWPF::UI::Thickness(1, 2, 3, 4));
    cppTree->SetSpacing(6);

    const auto* xamlElement = dynamic_cast<const NeoWPF::UI::FrameworkElement*>(xamlTree.get());
    ASSERT_NE(xamlElement, nullptr);
    EXPECT_EQ(xamlElement->GetWidth(), cppTree->GetWidth());
    EXPECT_EQ(xamlElement->GetHeight(), cppTree->GetHeight());
    EXPECT_EQ(xamlElement->GetMargin(), cppTree->GetMargin());
    EXPECT_EQ(xamlElement->GetSpacing(), cppTree->GetSpacing());
    EXPECT_EQ(std::any_cast<double>(xamlElement->GetRawValue(NeoWPF::UI::FrameworkElement::WidthProperty())),
              std::any_cast<double>(cppTree->GetRawValue(NeoWPF::UI::FrameworkElement::WidthProperty())));
    EXPECT_EQ(std::any_cast<NeoWPF::UI::Thickness>(xamlElement->GetRawValue(NeoWPF::UI::FrameworkElement::MarginProperty())),
              std::any_cast<NeoWPF::UI::Thickness>(cppTree->GetRawValue(NeoWPF::UI::FrameworkElement::MarginProperty())));

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchControlAppearanceProperties)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Button Text="Run" Foreground="#FF112233" Background="#FF445566" BorderBrush="#FF778899" Padding="2,3,4,5" FontSize="18" />)");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);
    auto* xamlButton = dynamic_cast<NeoWPF::UI::Button*>(xamlTree.get());
    ASSERT_NE(xamlButton, nullptr);

    auto cppButton = std::make_shared<NeoWPF::UI::Button>();
    cppButton->SetText(L"Run");
    cppButton->SetForeground(MakeSolidColorBrush(0xFF112233u));
    cppButton->SetBackgroundBrush(MakeSolidColorBrush(0xFF445566u));
    cppButton->SetBorderBrush(MakeSolidColorBrush(0xFF778899u));
    cppButton->SetPadding(NeoWPF::UI::Thickness(2, 3, 4, 5));
    cppButton->SetFontSize(18);

    EXPECT_EQ(xamlButton->GetText(), cppButton->GetText());
    ExpectSolidColorBrush(xamlButton->GetForeground(), 0xFF112233u);
    ExpectSolidColorBrush(cppButton->GetForeground(), 0xFF112233u);
    ExpectSolidColorBrush(xamlButton->GetBackgroundBrush(), 0xFF445566u);
    ExpectSolidColorBrush(cppButton->GetBackgroundBrush(), 0xFF445566u);
    ExpectSolidColorBrush(xamlButton->GetBorderBrush(), 0xFF778899u);
    ExpectSolidColorBrush(cppButton->GetBorderBrush(), 0xFF778899u);
    EXPECT_EQ(xamlButton->GetPadding(), cppButton->GetPadding());
    EXPECT_EQ(xamlButton->GetFontSize(), cppButton->GetFontSize());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchCheckboxStateAndPadding)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Checkbox Text="Accept" IsChecked="True" Padding="1,2,3,4" FontSize="16" />)");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);
    auto* xamlCheckbox = dynamic_cast<NeoWPF::UI::Checkbox*>(xamlTree.get());
    ASSERT_NE(xamlCheckbox, nullptr);

    auto cppCheckbox = std::make_shared<NeoWPF::UI::Checkbox>();
    cppCheckbox->SetText(L"Accept");
    cppCheckbox->SetChecked(true);
    cppCheckbox->SetPadding(NeoWPF::UI::Thickness(1, 2, 3, 4));
    cppCheckbox->SetFontSize(16);

    EXPECT_EQ(xamlCheckbox->GetText(), cppCheckbox->GetText());
    EXPECT_EQ(xamlCheckbox->IsChecked(), cppCheckbox->IsChecked());
    EXPECT_EQ(xamlCheckbox->GetPadding(), cppCheckbox->GetPadding());
    EXPECT_EQ(xamlCheckbox->GetFontSize(), cppCheckbox->GetFontSize());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchGridAttachedProperties)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Grid><Button Grid.Row="1" Grid.Column="2" Grid.RowSpan="2" Grid.ColumnSpan="3" /></Grid>)");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);
    auto* xamlGrid = dynamic_cast<NeoWPF::UI::Grid*>(xamlTree.get());
    ASSERT_NE(xamlGrid, nullptr);
    auto* xamlButton = dynamic_cast<NeoWPF::UI::Button*>(xamlGrid->GetVisualChildren()[0]);
    ASSERT_NE(xamlButton, nullptr);

    auto cppGrid = std::make_shared<NeoWPF::UI::Grid>();
    auto cppButton = std::make_shared<NeoWPF::UI::Button>();
    cppGrid->AddChild(cppButton, 1, 2, 2, 3);

    EXPECT_EQ(xamlGrid->GetRow(xamlButton), cppGrid->GetRow(cppButton.get()));
    EXPECT_EQ(xamlGrid->GetColumn(xamlButton), cppGrid->GetColumn(cppButton.get()));
    EXPECT_EQ(NeoWPF::UI::Grid::GetRowSpan(xamlButton), NeoWPF::UI::Grid::GetRowSpan(cppButton.get()));
    EXPECT_EQ(NeoWPF::UI::Grid::GetColumnSpan(xamlButton), NeoWPF::UI::Grid::GetColumnSpan(cppButton.get()));

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchGridDefinitionValues)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Grid><Grid.RowDefinitions><RowDefinition Height="24" /><RowDefinition Height="*" /></Grid.RowDefinitions><Grid.ColumnDefinitions><ColumnDefinition Width="2*" /></Grid.ColumnDefinitions></Grid>)");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);
    auto* xamlGrid = dynamic_cast<NeoWPF::UI::Grid*>(xamlTree.get());
    ASSERT_NE(xamlGrid, nullptr);

    auto cppGrid = std::make_shared<NeoWPF::UI::Grid>();
    auto row = std::make_shared<NeoWPF::UI::RowDefinition>();
    row->SetHeight(NeoWPF::UI::Grid::GridLength::Fixed(24));
    auto starRow = std::make_shared<NeoWPF::UI::RowDefinition>();
    starRow->SetHeight(NeoWPF::UI::Grid::GridLength::Star());
    auto column = std::make_shared<NeoWPF::UI::ColumnDefinition>();
    column->SetWidth(NeoWPF::UI::Grid::GridLength::Star(2));
    cppGrid->AddRowDefinition(row);
    cppGrid->AddRowDefinition(starRow);
    cppGrid->AddColumnDefinition(column);

    EXPECT_EQ(xamlGrid->GetVisualChildren().size(), 0u);
    EXPECT_EQ(row->GetHeight().Mode, NeoWPF::UI::SizeMode::Fixed);
    EXPECT_EQ(row->GetHeight().Value, 24);
    EXPECT_EQ(starRow->GetHeight().Mode, NeoWPF::UI::SizeMode::Star);
    EXPECT_EQ(column->GetWidth().Mode, NeoWPF::UI::SizeMode::Star);
    EXPECT_EQ(column->GetWidth().Value, 2);
    ASSERT_EQ(xamlGrid->GetRowDefinitions().size(), cppGrid->GetRowDefinitions().size());
    ASSERT_EQ(xamlGrid->GetColumnDefinitions().size(), cppGrid->GetColumnDefinitions().size());
    for (std::size_t index = 0; index < xamlGrid->GetRowDefinitions().size(); ++index) {
        EXPECT_EQ(xamlGrid->GetRowDefinitions()[index].mode, cppGrid->GetRowDefinitions()[index].mode);
        EXPECT_EQ(xamlGrid->GetRowDefinitions()[index].value, cppGrid->GetRowDefinitions()[index].value);
    }
    for (std::size_t index = 0; index < xamlGrid->GetColumnDefinitions().size(); ++index) {
        EXPECT_EQ(xamlGrid->GetColumnDefinitions()[index].mode, cppGrid->GetColumnDefinitions()[index].mode);
        EXPECT_EQ(xamlGrid->GetColumnDefinitions()[index].value, cppGrid->GetColumnDefinitions()[index].value);
    }

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchNestedGridLayout)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <Grid Width="600" Height="300">
            <Grid.RowDefinitions>
                <RowDefinition Height="Auto" />
                <RowDefinition Height="*" />
            </Grid.RowDefinitions>
            <Grid.ColumnDefinitions>
                <ColumnDefinition Width="180" />
                <ColumnDefinition Width="*" />
            </Grid.ColumnDefinitions>
            <Border Grid.Row="0" Grid.Column="0" Grid.ColumnSpan="2" Padding="8">
                <TextBlock Text="Header" />
            </Border>
            <StackPanel Grid.Row="1" Grid.Column="0" Spacing="4">
                <Button Text="One" />
                <Button Text="Two" />
            </StackPanel>
            <TextBox Grid.Row="1" Grid.Column="1" Text="Input" PlaceholderText="Type" />
        </Grid>
    )");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);
    auto* xamlGrid = dynamic_cast<NeoWPF::UI::Grid*>(xamlTree.get());
    ASSERT_NE(xamlGrid, nullptr);

    auto cppGrid = std::make_shared<NeoWPF::UI::Grid>();
    cppGrid->SetWidth(600);
    cppGrid->SetHeight(300);
    cppGrid->AddRows({{NeoWPF::UI::SizeMode::Auto, 0}, {NeoWPF::UI::SizeMode::Star, 1}});
    cppGrid->AddColumns({{NeoWPF::UI::SizeMode::Fixed, 180}, {NeoWPF::UI::SizeMode::Star, 1}});
    auto cppBorder = std::make_shared<NeoWPF::UI::Border>();
    cppBorder->SetPadding(8);
    cppBorder->SetChild(std::make_shared<NeoWPF::UI::TextBlock>(L"Header"));
    auto cppStack = std::make_shared<NeoWPF::UI::StackPanel>();
    cppStack->SetSpacing(4);
    cppStack->AddChild(std::make_shared<NeoWPF::UI::Button>(L"One"));
    cppStack->AddChild(std::make_shared<NeoWPF::UI::Button>(L"Two"));
    auto cppTextBox = std::make_shared<NeoWPF::UI::TextBox>();
    cppTextBox->SetText(L"Input");
    cppTextBox->SetPlaceholderText(L"Type");
    cppGrid->AddChild(cppBorder, 0, 0, 1, 2);
    cppGrid->AddChild(cppStack, 1, 0);
    cppGrid->AddChild(cppTextBox, 1, 1);

    ASSERT_EQ(xamlGrid->GetRowDefinitions().size(), 2u);
    ASSERT_EQ(cppGrid->GetRowDefinitions().size(), 2u);
    ASSERT_EQ(xamlGrid->GetColumnDefinitions().size(), 2u);
    ASSERT_EQ(cppGrid->GetColumnDefinitions().size(), 2u);
    EXPECT_EQ(xamlGrid->GetVisualChildren().size(), cppGrid->GetVisualChildren().size());
    const auto xamlDescendants = xamlGrid->GetVisualDescendants();
    const auto cppDescendants = cppGrid->GetVisualDescendants();
    ASSERT_EQ(xamlDescendants.size(), cppDescendants.size());
    for (std::size_t index = 0; index < xamlDescendants.size(); ++index) {
        const auto* const xamlDescendant = xamlDescendants[index];
        const auto* const cppDescendant = cppDescendants[index];
        EXPECT_EQ(typeid(*xamlDescendant), typeid(*cppDescendant));
    }

    const auto xamlChildren = xamlGrid->GetVisualChildren();
    const auto cppChildren = cppGrid->GetVisualChildren();
    ASSERT_EQ(xamlChildren.size(), 3u);
    ASSERT_EQ(cppChildren.size(), 3u);
    for (std::size_t index = 0; index < xamlChildren.size(); ++index) {
        EXPECT_EQ(xamlGrid->GetRow(xamlChildren[index]), cppGrid->GetRow(cppChildren[index]));
        EXPECT_EQ(xamlGrid->GetColumn(xamlChildren[index]), cppGrid->GetColumn(cppChildren[index]));
        EXPECT_EQ(NeoWPF::UI::Grid::GetRowSpan(xamlChildren[index]),
                  NeoWPF::UI::Grid::GetRowSpan(cppChildren[index]));
        EXPECT_EQ(NeoWPF::UI::Grid::GetColumnSpan(xamlChildren[index]),
                  NeoWPF::UI::Grid::GetColumnSpan(cppChildren[index]));
    }

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchContentControlNesting)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Border Padding="5"><Border Padding="3"><TextBlock Text="Nested" /></Border></Border>)");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);

    auto cppOuter = std::make_shared<NeoWPF::UI::Border>();
    auto cppInner = std::make_shared<NeoWPF::UI::Border>();
    cppOuter->SetPadding(5);
    cppInner->SetPadding(3);
    cppInner->SetChild(std::make_shared<NeoWPF::UI::TextBlock>(L"Nested"));
    cppOuter->SetChild(cppInner);

    const auto xamlDescendants = AsVisual(xamlTree)->GetVisualDescendants();
    const auto cppDescendants = cppOuter->GetVisualDescendants();
    ASSERT_EQ(xamlDescendants.size(), cppDescendants.size());
    for (std::size_t index = 0; index < xamlDescendants.size(); ++index) {
        const auto* const xamlDescendant = xamlDescendants[index];
        const auto* const cppDescendant = cppDescendants[index];
        EXPECT_EQ(typeid(*xamlDescendant), typeid(*cppDescendant));
    }

    const auto* xamlOuter = dynamic_cast<const NeoWPF::UI::Border*>(xamlTree.get());
    ASSERT_NE(xamlOuter, nullptr);
    EXPECT_EQ(xamlOuter->GetPadding(), cppOuter->GetPadding());
    const auto* xamlInner = dynamic_cast<const NeoWPF::UI::Border*>(xamlOuter->GetChild().get());
    const auto* cppInnerValue = dynamic_cast<const NeoWPF::UI::Border*>(cppOuter->GetChild().get());
    ASSERT_NE(xamlInner, nullptr);
    ASSERT_NE(cppInnerValue, nullptr);
    EXPECT_EQ(xamlInner->GetPadding(), cppInnerValue->GetPadding());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchArrangedPaddedGrid)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <Grid Width="420" Height="220" Padding="10,5,10,5" Spacing="6,4">
            <Grid.RowDefinitions>
                <RowDefinition Height="40" />
                <RowDefinition Height="*" />
            </Grid.RowDefinitions>
            <Grid.ColumnDefinitions>
                <ColumnDefinition Width="120" />
                <ColumnDefinition Width="*" />
                <ColumnDefinition Width="80" />
            </Grid.ColumnDefinitions>
            <TextBlock Grid.Row="0" Grid.Column="0" Text="Title" Margin="2,3,4,5" />
            <TextBox Grid.Row="1" Grid.Column="0" Grid.ColumnSpan="2" Text="Query" Padding="3,4,5,6" />
            <Checkbox Grid.Row="1" Grid.Column="2" Text="Ready" IsChecked="True" />
        </Grid>
    )");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);

    auto cppGrid = std::make_shared<NeoWPF::UI::Grid>();
    cppGrid->SetWidth(420);
    cppGrid->SetHeight(220);
    cppGrid->SetPadding(NeoWPF::UI::Thickness(10, 5, 10, 5));
    cppGrid->SetSpacing(NeoWPF::UI::Spacing(6, 4));
    cppGrid->AddRows({{NeoWPF::UI::SizeMode::Fixed, 40}, {NeoWPF::UI::SizeMode::Star, 1}});
    cppGrid->AddColumns({{NeoWPF::UI::SizeMode::Fixed, 120}, {NeoWPF::UI::SizeMode::Star, 1}, {NeoWPF::UI::SizeMode::Fixed, 80}});

    auto cppTitle = std::make_shared<NeoWPF::UI::TextBlock>(L"Title");
    cppTitle->SetMargin(NeoWPF::UI::Thickness(2, 3, 4, 5));
    auto cppInput = std::make_shared<NeoWPF::UI::TextBox>();
    cppInput->SetText(L"Query");
    cppInput->SetPadding(NeoWPF::UI::Thickness(3, 4, 5, 6));
    auto cppReady = std::make_shared<NeoWPF::UI::Checkbox>();
    cppReady->SetText(L"Ready");
    cppReady->SetChecked(true);
    cppGrid->AddChild(cppTitle, 0, 0);
    cppGrid->AddChild(cppInput, 1, 0, 1, 2);
    cppGrid->AddChild(cppReady, 1, 2);

    auto* xamlGrid = dynamic_cast<NeoWPF::UI::Grid*>(xamlTree.get());
    ASSERT_NE(xamlGrid, nullptr);
    xamlGrid->Measure(420, 220);
    xamlGrid->Arrange(420, 220);
    cppGrid->Measure(420, 220);
    cppGrid->Arrange(420, 220);

    EXPECT_EQ(xamlGrid->GetDesiredWidth(), cppGrid->GetDesiredWidth());
    EXPECT_EQ(xamlGrid->GetDesiredHeight(), cppGrid->GetDesiredHeight());
    EXPECT_EQ(xamlGrid->GetFinalWidth(), cppGrid->GetFinalWidth());
    EXPECT_EQ(xamlGrid->GetFinalHeight(), cppGrid->GetFinalHeight());
    ASSERT_EQ(xamlGrid->GetRowDefinitions().size(), cppGrid->GetRowDefinitions().size());
    ASSERT_EQ(xamlGrid->GetColumnDefinitions().size(), cppGrid->GetColumnDefinitions().size());
    EXPECT_EQ(xamlGrid->GetVisualChildren().size(), cppGrid->GetVisualChildren().size());
    ExpectLayoutParity(*xamlGrid, *cppGrid);

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchNestedStackPanelArrangement)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <StackPanel Width="360" Height="180" Spacing="8">
            <StackPanel Width="140" Margin="4,6,8,10" Spacing="3">
                <Button Text="A" Height="32" />
                <Button Text="B" Height="36" />
            </StackPanel>
            <Border Width="180" Padding="7,5,3,2">
                <TextBlock Text="Panel" Margin="2,1,4,3" />
            </Border>
        </StackPanel>
    )");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);

    auto cppRoot = std::make_shared<NeoWPF::UI::StackPanel>();
    cppRoot->SetWidth(360);
    cppRoot->SetHeight(180);
    cppRoot->SetSpacing(8);
    auto cppStack = std::make_shared<NeoWPF::UI::StackPanel>();
    cppStack->SetWidth(140);
    cppStack->SetMargin(NeoWPF::UI::Thickness(4, 6, 8, 10));
    cppStack->SetSpacing(3);
    auto cppFirstButton = std::make_shared<NeoWPF::UI::Button>(L"A");
    cppFirstButton->SetHeight(32);
    auto cppSecondButton = std::make_shared<NeoWPF::UI::Button>(L"B");
    cppSecondButton->SetHeight(36);
    cppStack->AddChild(cppFirstButton);
    cppStack->AddChild(cppSecondButton);
    auto cppBorder = std::make_shared<NeoWPF::UI::Border>();
    cppBorder->SetWidth(180);
    cppBorder->SetPadding(NeoWPF::UI::Thickness(7, 5, 3, 2));
    auto cppPanelText = std::make_shared<NeoWPF::UI::TextBlock>(L"Panel");
    cppPanelText->SetMargin(NeoWPF::UI::Thickness(2, 1, 4, 3));
    cppBorder->SetChild(cppPanelText);
    cppRoot->AddChild(cppStack);
    cppRoot->AddChild(cppBorder);

    auto* xamlPanel = dynamic_cast<NeoWPF::UI::StackPanel*>(xamlTree.get());
    ASSERT_NE(xamlPanel, nullptr);
    xamlPanel->Measure(360, 180);
    xamlPanel->Arrange(360, 180);
    cppRoot->Measure(360, 180);
    cppRoot->Arrange(360, 180);

    ExpectLayoutParity(*xamlPanel, *cppRoot);
    EXPECT_EQ(xamlPanel->GetVisualChildren().size(), cppRoot->GetVisualChildren().size());
    EXPECT_EQ(xamlPanel->GetDesiredWidth(), cppRoot->GetDesiredWidth());
    EXPECT_EQ(xamlPanel->GetDesiredHeight(), cppRoot->GetDesiredHeight());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, RaspyCppAndXamlLayoutsMatchEverySemanticNode)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <Grid Background="#FF0B1324" Padding="32">
            <Grid.RowDefinitions>
                <RowDefinition Height="Auto" />
                <RowDefinition Height="*" />
                <RowDefinition Height="Auto" />
            </Grid.RowDefinitions>
            <StackPanel Grid.Row="0" Margin="0,0,0,24">
                <TextBlock Text="NeoWPF Binding Laboratory" FontSize="32" Foreground="#FFF8FAFC" />
                <TextBlock Text="Edit a bound value and watch the preview answer immediately." FontSize="15" Foreground="#FF9FB0C7" Margin="0,7,0,0" />
            </StackPanel>
            <Grid Grid.Row="1">
                <Grid.ColumnDefinitions>
                    <ColumnDefinition Width="320" />
                    <ColumnDefinition Width="*" />
                </Grid.ColumnDefinitions>
                <Border Grid.Column="0" Background="#FF16233A" BorderBrush="#FF2D4263" BorderThickness="1" Padding="20" Margin="0,0,24,0">
                    <StackPanel>
                        <TextBlock Text="Change the input" FontSize="20" Foreground="#FFE5EDF8" Margin="0,0,0,18" />
                        <TextBlock Text="The values below are TwoWay bindings." FontSize="13" Foreground="#FF9FB0C7" Margin="0,0,0,18" />
                        <TextBlock Text="Message" FontSize="12" Foreground="#FF9FB0C7" />
                        <TextBox Text="Hello from the view model" PlaceholderText="Type a message" Height="40" Margin="0,6,0,20" />
                        <Checkbox Text="Enable live preview" IsChecked="True" Margin="0,0,0,22" />
                        <Button Text="Apply binding update" Height="42" />
                    </StackPanel>
                </Border>
                <Border Grid.Column="1" Background="#FF12304A" BorderBrush="#FF5EEAD4" BorderThickness="2" Padding="28">
                    <StackPanel VerticalAlignment="Center">
                        <TextBlock Text="LIVE PREVIEW" FontSize="12" Foreground="#FF5EEAD4" />
                        <TextBlock Text="Preview is active" FontSize="34" Foreground="#FFF8FAFC" Margin="0,10,0,8" />
                        <TextBlock Text="This panel is driven by properties on the bound view model." FontSize="17" Foreground="#FFD7E2F2" TextWrapping="Wrap" />
                        <TextBlock Text="Hello from the view model" FontSize="22" Foreground="#FF5EEAD4" Margin="0,24,0,0" />
                        <TextBlock Text="Bindings are connected" FontSize="14" Foreground="#FF9FB0C7" Margin="0,8,0,0" />
                    </StackPanel>
                </Border>
            </Grid>
            <Grid Grid.Row="2" Margin="0,24,0,0">
                <Grid.ColumnDefinitions>
                    <ColumnDefinition Width="*" />
                    <ColumnDefinition Width="Auto" />
                </Grid.ColumnDefinitions>
                <TextBlock Grid.Column="0" Text="TextBox and Checkbox use TwoWay bindings" FontSize="13" Foreground="#FF9FB0C7" />
                <TextBlock Grid.Column="1" Text="Initial snapshot" FontSize="13" Foreground="#FF7183A1" />
            </Grid>
        </Grid>
    )");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);

    auto content = std::make_shared<NeoWPF::UI::Grid>();
    content->SetBackgroundBrush(MakeSolidColorBrush(0xFF0B1324u));
    content->SetPadding(32);
    content->AddRow(NeoWPF::UI::SizeMode::Auto);
    content->AddRow(NeoWPF::UI::SizeMode::Star, 1);
    content->AddRow(NeoWPF::UI::SizeMode::Auto);

    auto header = std::make_shared<NeoWPF::UI::StackPanel>();
    header->SetMargin(0, 0, 0, 24);
    auto title = std::make_shared<NeoWPF::UI::TextBlock>(L"NeoWPF Binding Laboratory");
    title->SetFontSize(32); title->SetForeground(MakeSolidColorBrush(0xFFF8FAFCu)); header->AddChild(title);
    auto subtitle = std::make_shared<NeoWPF::UI::TextBlock>(L"Edit a bound value and watch the preview answer immediately.");
    subtitle->SetFontSize(15); subtitle->SetForeground(MakeSolidColorBrush(0xFF9FB0C7u)); subtitle->SetMargin(0, 7, 0, 0); header->AddChild(subtitle);
    content->AddChild(header, 0, 0);

    auto body = std::make_shared<NeoWPF::UI::Grid>();
    body->AddColumn(NeoWPF::UI::SizeMode::Fixed, 320);
    body->AddColumn(NeoWPF::UI::SizeMode::Star, 1);
    auto leftBorder = std::make_shared<NeoWPF::UI::Border>();
    leftBorder->SetBackgroundBrush(MakeSolidColorBrush(0xFF16233Au)); leftBorder->SetBorderBrush(MakeSolidColorBrush(0xFF2D4263u));
    leftBorder->SetBorderThickness(NeoWPF::UI::Thickness(1)); leftBorder->SetPadding(20); leftBorder->SetMargin(0, 0, 24, 0);
    auto leftStack = std::make_shared<NeoWPF::UI::StackPanel>();
    auto inputTitle = std::make_shared<NeoWPF::UI::TextBlock>(L"Change the input");
    inputTitle->SetFontSize(20); inputTitle->SetForeground(MakeSolidColorBrush(0xFFE5EDF8u)); inputTitle->SetMargin(0, 0, 0, 18); leftStack->AddChild(inputTitle);
    auto inputDescription = std::make_shared<NeoWPF::UI::TextBlock>(L"The values below are TwoWay bindings.");
    inputDescription->SetFontSize(13); inputDescription->SetForeground(MakeSolidColorBrush(0xFF9FB0C7u)); inputDescription->SetMargin(0, 0, 0, 18); leftStack->AddChild(inputDescription);
    auto messageLabel = std::make_shared<NeoWPF::UI::TextBlock>(L"Message");
    messageLabel->SetFontSize(12); messageLabel->SetForeground(MakeSolidColorBrush(0xFF9FB0C7u)); leftStack->AddChild(messageLabel);
    auto queryBox = std::make_shared<NeoWPF::UI::TextBox>();
    queryBox->SetText(L"Hello from the view model"); queryBox->SetPlaceholderText(L"Type a message"); queryBox->SetHeight(40); queryBox->SetMargin(0, 6, 0, 20); leftStack->AddChild(queryBox);
    auto checkbox = std::make_shared<NeoWPF::UI::Checkbox>();
    checkbox->SetText(L"Enable live preview"); checkbox->SetChecked(true); checkbox->SetMargin(0, 0, 0, 22); leftStack->AddChild(checkbox);
    auto applyButton = std::make_shared<NeoWPF::UI::Button>(L"Apply binding update"); applyButton->SetHeight(42); leftStack->AddChild(applyButton);
    leftBorder->SetChild(leftStack); body->AddChild(leftBorder, 0, 0);

    auto previewBorder = std::make_shared<NeoWPF::UI::Border>();
    previewBorder->SetBackgroundBrush(MakeSolidColorBrush(0xFF12304Au)); previewBorder->SetBorderBrush(MakeSolidColorBrush(0xFF5EEAD4u)); previewBorder->SetBorderThickness(NeoWPF::UI::Thickness(2)); previewBorder->SetPadding(28);
    auto previewStack = std::make_shared<NeoWPF::UI::StackPanel>(); previewStack->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    auto previewLabel = std::make_shared<NeoWPF::UI::TextBlock>(L"LIVE PREVIEW"); previewLabel->SetFontSize(12); previewLabel->SetForeground(MakeSolidColorBrush(0xFF5EEAD4u)); previewStack->AddChild(previewLabel);
    auto previewTitle = std::make_shared<NeoWPF::UI::TextBlock>(L"Preview is active"); previewTitle->SetFontSize(34); previewTitle->SetForeground(MakeSolidColorBrush(0xFFF8FAFCu)); previewTitle->SetMargin(0, 10, 0, 8); previewStack->AddChild(previewTitle);
    auto previewDetail = std::make_shared<NeoWPF::UI::TextBlock>(L"This panel is driven by properties on the bound view model."); previewDetail->SetFontSize(17); previewDetail->SetForeground(MakeSolidColorBrush(0xFFD7E2F2u)); previewDetail->SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap); previewStack->AddChild(previewDetail);
    auto queryPreview = std::make_shared<NeoWPF::UI::TextBlock>(L"Hello from the view model"); queryPreview->SetFontSize(22); queryPreview->SetForeground(MakeSolidColorBrush(0xFF5EEAD4u)); queryPreview->SetMargin(0, 24, 0, 0); previewStack->AddChild(queryPreview);
    auto status = std::make_shared<NeoWPF::UI::TextBlock>(L"Bindings are connected"); status->SetFontSize(14); status->SetForeground(MakeSolidColorBrush(0xFF9FB0C7u)); status->SetMargin(0, 8, 0, 0); previewStack->AddChild(status);
    previewBorder->SetChild(previewStack); body->AddChild(previewBorder, 0, 1); content->AddChild(body, 1, 0);

    auto footerGrid = std::make_shared<NeoWPF::UI::Grid>(); footerGrid->AddColumn(NeoWPF::UI::SizeMode::Star, 1); footerGrid->AddColumn(NeoWPF::UI::SizeMode::Auto); footerGrid->SetMargin(0, 24, 0, 0);
    auto footer = std::make_shared<NeoWPF::UI::TextBlock>(L"TextBox and Checkbox use TwoWay bindings"); footer->SetFontSize(13); footer->SetForeground(MakeSolidColorBrush(0xFF9FB0C7u)); footerGrid->AddChild(footer, 0, 0);
    auto lastUpdated = std::make_shared<NeoWPF::UI::TextBlock>(L"Initial snapshot"); lastUpdated->SetFontSize(13); lastUpdated->SetForeground(MakeSolidColorBrush(0xFF7183A1u)); footerGrid->AddChild(lastUpdated, 0, 1); content->AddChild(footerGrid, 2, 0);

    auto* xamlGrid = dynamic_cast<NeoWPF::UI::Grid*>(xamlTree.get());
    ASSERT_NE(xamlGrid, nullptr);
    xamlGrid->Measure(1180, 760); xamlGrid->Arrange(1180, 760);
    content->Measure(1180, 760); content->Arrange(1180, 760);

    ExpectLayoutParity(*xamlGrid, *content);
    ASSERT_EQ(xamlGrid->GetVisualChildren().size(), content->GetVisualChildren().size());
    ASSERT_EQ(xamlGrid->GetVisualDescendants().size(), content->GetVisualDescendants().size());

    const auto xamlChildren = xamlGrid->GetVisualChildren();
    const auto cppChildren = content->GetVisualChildren();
    for (std::size_t index = 0; index < xamlChildren.size(); ++index) {
        EXPECT_EQ(xamlGrid->GetRow(xamlChildren[index]), content->GetRow(cppChildren[index]));
        EXPECT_EQ(xamlGrid->GetColumn(xamlChildren[index]), content->GetColumn(cppChildren[index]));
    }

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchBorderContent)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Border Padding="4"><TextBlock Text="Inside" /></Border>)");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);
    auto* xamlBorder = dynamic_cast<NeoWPF::UI::Border*>(xamlTree.get());
    ASSERT_NE(xamlBorder, nullptr);
    ASSERT_NE(xamlBorder->GetChild(), nullptr);

    auto cppBorder = std::make_shared<NeoWPF::UI::Border>();
    auto cppText = std::make_shared<NeoWPF::UI::TextBlock>(L"Inside");
    cppBorder->SetPadding(4);
    cppBorder->SetChild(cppText);

    auto* xamlText = dynamic_cast<NeoWPF::UI::TextBlock*>(xamlBorder->GetChild().get());
    ASSERT_NE(xamlText, nullptr);
    EXPECT_EQ(xamlText->GetText(), dynamic_cast<NeoWPF::UI::TextBlock*>(cppBorder->GetChild().get())->GetText());
    EXPECT_EQ(xamlBorder->GetPadding(), cppBorder->GetPadding());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, XamlAndCppMatchTextBoxValues)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<TextBox Text="Query" PlaceholderText="Search" FontSize="15" />)");
    const auto xamlTree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(xamlTree, nullptr);
    auto* xamlTextBox = dynamic_cast<NeoWPF::UI::TextBox*>(xamlTree.get());
    ASSERT_NE(xamlTextBox, nullptr);

    auto cppTextBox = std::make_shared<NeoWPF::UI::TextBox>();
    cppTextBox->SetText(L"Query");
    cppTextBox->SetPlaceholderText(L"Search");
    cppTextBox->SetFontSize(15);

    EXPECT_EQ(xamlTextBox->GetText(), cppTextBox->GetText());
    EXPECT_EQ(xamlTextBox->GetPlaceholderText(), cppTextBox->GetPlaceholderText());
    EXPECT_EQ(xamlTextBox->GetFontSize(), cppTextBox->GetFontSize());

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, ReportsUnknownTypeAsDeveloperError)
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::ClearDeveloperWarnings();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml("<UnknownControl />");
    EXPECT_EQ(NeoWPF::UI::Markup::Xaml::BuildXamlTree(root), nullptr);

    const auto warning = NeoWPF::Diagnostics::GetLastWarning();
    ASSERT_TRUE(warning.has_value());
    EXPECT_EQ(warning->title, L"NEO-XAML103");
    EXPECT_EQ(warning->severity, NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, ReportsUnknownAttributeAsDeveloperError)
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::ClearDeveloperWarnings();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml("<TextBlock UnknownAttribute=\"value\" />");
    ASSERT_NE(NeoWPF::UI::Markup::Xaml::BuildXamlTree(root), nullptr);

    const auto warning = NeoWPF::Diagnostics::GetLastWarning();
    ASSERT_TRUE(warning.has_value());
    EXPECT_EQ(warning->title, L"NEO-XAML101");
    EXPECT_EQ(warning->severity, NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);

    NeoWPF::Core::Application::Shutdown();
}

TEST(XamlParserTests, ReportsUnboundPropertyElementAsDeveloperError)
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::ClearDeveloperWarnings();

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <Grid>
            <Grid.Width>
                <TextBlock />
            </Grid.Width>
        </Grid>
    )");
    ASSERT_NE(NeoWPF::UI::Markup::Xaml::BuildXamlTree(root), nullptr);

    const auto warning = NeoWPF::Diagnostics::GetLastWarning();
    ASSERT_TRUE(warning.has_value());
    EXPECT_EQ(warning->title, L"NEO-XAML105");
    EXPECT_EQ(warning->severity, NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);

    NeoWPF::Core::Application::Shutdown();
}
