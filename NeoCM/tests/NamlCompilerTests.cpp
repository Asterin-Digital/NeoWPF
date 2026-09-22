#include <gtest/gtest.h>

#include <NeoCM/Core/Compiler/CppEmitter.h>
#include <NeoCM/Core/Compiler/NamlCompiler.h>
#include <NeoCM/Core/Compiler/ResolvedXaml.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <unordered_set>

namespace NeoCM::Core::Compiler {
namespace {

TEST(NamlCompilerTests, CompilesRootAndNestedElements)
{
    const auto result = CompileXaml(
        "<Grid><Button Text=\"Apply\" /><TextBlock Text=\"{Binding Title, Mode=TwoWay}\" /></Grid>",
        "MainWindow.xaml");

    ASSERT_TRUE(result.success) << result.error;
    ASSERT_EQ(result.artifact.objects.size(), 3u);
    ASSERT_EQ(result.artifact.children.size(), 2u);
    ASSERT_EQ(result.artifact.bindings.size(), 1u);
    EXPECT_EQ(result.artifact.objects[0].id, result.artifact.rootObjectId);
    EXPECT_EQ(result.artifact.objects[0].name, "");
    EXPECT_EQ(result.artifact.children[0].parentId, 0u);
    EXPECT_EQ(result.artifact.children[0].childId, 1u);
    EXPECT_EQ(result.artifact.bindings[0].objectId, 2u);
    EXPECT_EQ(result.artifact.bindings[0].path, "Title");
    EXPECT_EQ(result.artifact.bindings[0].mode, "TwoWay");
}

TEST(NamlCompilerTests, PreservesNamedElementsInArtifact)
{
    const auto result = CompileXaml(
        "<Grid x:Name=\"RootGrid\"><Button x:Name=\"ApplyButton\" /></Grid>",
        "MainWindow.xaml");

    ASSERT_TRUE(result.success) << result.error;
    ASSERT_EQ(result.artifact.objects.size(), 2u);
    EXPECT_EQ(result.artifact.objects[0].name, "RootGrid");
    EXPECT_EQ(result.artifact.objects[1].name, "ApplyButton");
}

TEST(NamlCompilerTests, CompilesResourceDictionaryEntriesWithKeys)
{
    const auto result = CompileXaml(
        "<Grid><Grid.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" Text=\"Apply\" /></ResourceDictionary></Grid.Resources></Grid>",
        "Resources.xaml");

    ASSERT_TRUE(result.success) << result.error;
    ASSERT_EQ(result.artifact.objects.size(), 2u);
    EXPECT_EQ(result.artifact.objects[1].resourceKey, "Accent");
    ASSERT_EQ(result.artifact.children.size(), 1u);
    EXPECT_EQ(result.artifact.children[0].propertyString < result.artifact.strings.size(), true);
    EXPECT_EQ(result.artifact.strings[result.artifact.children[0].propertyString], "Resources");
}

TEST(NamlCompilerTests, CompilesKeylessImplicitStyleResource)
{
    const auto result = CompileXaml(
        "<Grid><Grid.Resources><ResourceDictionary><Style TargetType=\"Button\"><Setter Property=\"Text\" Value=\"Compiled implicit\" /></Style></ResourceDictionary></Grid.Resources></Grid>",
        "ImplicitStyle.xaml");

    ASSERT_TRUE(result.success) << result.error;
    ASSERT_EQ(result.artifact.objects.size(), 3u);
    EXPECT_EQ(result.artifact.objects[1].typeString < result.artifact.strings.size(), true);
    EXPECT_EQ(result.artifact.objects[1].resourceKey, "");
    EXPECT_EQ(result.artifact.objects[2].resourceKey, "");
    ASSERT_EQ(result.artifact.children.size(), 2u);
    std::unordered_set<std::string> childProperties;
    for (const auto& child : result.artifact.children) {
        childProperties.insert(result.artifact.strings[child.propertyString]);
    }
    EXPECT_TRUE(childProperties.contains("Resources"));
    EXPECT_TRUE(childProperties.contains("Setters"));
}

TEST(NamlCompilerTests, RejectsResourceEntryWithoutKey)
{
    const auto result = CompileXaml(
        "<Grid><Grid.Resources><ResourceDictionary><TextBlock Text=\"Apply\" /></ResourceDictionary></Grid.Resources></Grid>",
        "MissingResourceKey.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("missing x:Key"), std::string::npos);
}

TEST(NamlCompilerTests, RejectsDuplicateResourceKeys)
{
    const auto result = CompileXaml(
        "<Grid><Grid.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" /><TextBlock x:Key=\"Accent\" /></ResourceDictionary></Grid.Resources></Grid>",
        "DuplicateResourceKey.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("duplicate resource key 'Accent'"), std::string::npos);
}

TEST(NamlCompilerTests, EmitsStaticResourcePropertyRecord)
{
    const auto result = CompileXaml(
        "<Grid><Grid.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" Text=\"Compiled\" /></ResourceDictionary></Grid.Resources><TextBlock Text=\"{StaticResource Accent}\" /></Grid>",
        "StaticResource.xaml");

    ASSERT_TRUE(result.success) << result.error;
    ASSERT_EQ(result.artifact.objects.size(), 3u);
    ASSERT_EQ(result.artifact.properties.size(), 2u);
    EXPECT_EQ(result.artifact.objects[1].resourceKey, "Accent");
    EXPECT_EQ(result.artifact.properties[1].value.kind, NeoNaml::NamlValueKind::MarkupExtension);
    EXPECT_EQ(result.artifact.properties[1].value.text, "{StaticResource Accent}");
}

TEST(NamlCompilerTests, EmitsLocalePropertyRecord)
{
    const auto result = CompileXaml(
        "<TextBlock Text=\"{Locale Key=Greeting}\" />",
        "Locale.xaml");

    ASSERT_TRUE(result.success) << result.error;
    ASSERT_EQ(result.artifact.properties.size(), 1u);
    EXPECT_EQ(result.artifact.properties[0].value.kind, NeoNaml::NamlValueKind::MarkupExtension);
    EXPECT_EQ(result.artifact.properties[0].value.text, "{Locale Key=Greeting}");
}

TEST(NamlCompilerTests, AcceptsStaticResourceFromLocalScope)
{
    const auto result = CompileXaml(
        "<Grid><Grid.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" /></ResourceDictionary></Grid.Resources><TextBlock Text=\"{StaticResource Accent}\" /></Grid>",
        "LocalStaticResource.xaml");

    ASSERT_TRUE(result.success) << result.error;
}

TEST(NamlCompilerTests, AcceptsStaticResourceFromAncestorScope)
{
    const auto result = CompileXaml(
        "<Grid><Grid.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" /></ResourceDictionary></Grid.Resources><Border><TextBlock Text=\"{StaticResource Accent}\" /></Border></Grid>",
        "AncestorStaticResource.xaml");

    ASSERT_TRUE(result.success) << result.error;
}

TEST(NamlCompilerTests, AcceptsShadowedStaticResourceInNestedScope)
{
    const auto result = CompileXaml(
        "<Grid><Grid.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" /></ResourceDictionary></Grid.Resources><Border><Border.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" /></ResourceDictionary></Border.Resources><TextBlock Text=\"{StaticResource Accent}\" /></Border></Grid>",
        "ShadowedStaticResource.xaml");

    ASSERT_TRUE(result.success) << result.error;
}

TEST(NamlCompilerTests, RejectsMissingStaticResource)
{
    const auto result = CompileXaml(
        "<Grid><TextBlock Text=\"{StaticResource Missing}\" /></Grid>",
        "MissingStaticResource.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("StaticResource 'Missing' was not found in scope"), std::string::npos);
}

TEST(NamlCompilerTests, AcceptsListBoxItemContentAsTextValue)
{
    const auto result = CompileXaml(
        "<ListBox><ListBoxItem Content=\"Alpha\" /></ListBox>",
        "MainWindow.xaml");

    EXPECT_TRUE(result.success) << result.error;
    ASSERT_EQ(result.artifact.objects.size(), 2u);
    ASSERT_LT(result.artifact.objects[1].typeString, result.artifact.strings.size());
    EXPECT_EQ(result.artifact.strings[result.artifact.objects[1].typeString], "ListBoxItem");
}

TEST(ResolvedXamlTests, ResolvesMetadataAndChildRelationships)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Grid x:Name=\"RootGrid\"><Grid.RowDefinitions><RowDefinition /></Grid.RowDefinitions><Button x:Name=\"ApplyButton\" Text=\"Apply\" /></Grid>");

    const auto result = ResolveXaml(roots, "MainWindow.xaml");

    ASSERT_TRUE(result.success) << result.error;
    EXPECT_EQ(result.document.sourcePath, "MainWindow.xaml");
    EXPECT_EQ(result.document.root.tagName, "Grid");
    EXPECT_EQ(result.document.root.cppTypeName, "NeoWPF::UI::Grid");
    EXPECT_EQ(result.document.root.headerPath, "NeoWPF/UI/Layout/Grid/Grid.h");
    EXPECT_EQ(result.document.root.xamlName, "RootGrid");
    ASSERT_EQ(result.document.root.children.size(), 2u);
    EXPECT_EQ(result.document.root.children[0].propertyName, "RowDefinitions");
    EXPECT_EQ(result.document.root.children[0].propertyExpression,
              "NeoWPF::UI::Grid::RowDefinitionsProperty()");
    EXPECT_EQ(result.document.root.children[0].element.tagName, "RowDefinition");
    EXPECT_TRUE(result.document.root.children[1].propertyName.empty());
    EXPECT_EQ(result.document.root.children[1].element.xamlName, "ApplyButton");
    ASSERT_EQ(result.document.root.children[1].element.properties.size(), 1u);
    EXPECT_EQ(result.document.root.children[1].element.properties[0].name, "Text");
    EXPECT_EQ(result.document.root.children[1].element.properties[0].value, "Apply");
}

TEST(ResolvedXamlTests, PreservesStaticResourceMetadata)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Grid><Grid.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" /></ResourceDictionary></Grid.Resources><TextBlock Text=\"{StaticResource Accent}\" /></Grid>");

    const auto result = ResolveXaml(roots, "StaticResource.xaml");

    ASSERT_TRUE(result.success) << result.error;
    ASSERT_EQ(result.document.root.children.size(), 1u);
    ASSERT_EQ(result.document.root.children[0].element.properties.size(), 1u);
    const auto& property = result.document.root.children[0].element.properties[0];
    EXPECT_TRUE(property.isMarkupExtension);
    EXPECT_EQ(property.staticResourceKey, "Accent");
}

TEST(ResolvedXamlTests, PreservesLocaleMetadata)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<TextBlock Text=\"{Locale Key=Greeting}\" />");

    const auto result = ResolveXaml(roots, "Locale.xaml");

    ASSERT_TRUE(result.success) << result.error;
    ASSERT_EQ(result.document.root.properties.size(), 1u);
    const auto& property = result.document.root.properties[0];
    EXPECT_TRUE(property.isMarkupExtension);
    EXPECT_EQ(property.localeKey, "Greeting");
}

TEST(ResolvedXamlTests, PreservesRootClassMetadata)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Window x:Class="SampleApp.MainWindow" />)");

    const auto result = ResolveXaml(roots, "MainWindow.xaml");

    ASSERT_TRUE(result.success) << result.error;
    EXPECT_EQ(result.document.xamlClass, "SampleApp.MainWindow");
    EXPECT_EQ(result.document.root.cppTypeName, "NeoWPF::UI::Window");
}

TEST(ResolvedXamlTests, RejectsNestedClassMetadata)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Window><Grid x:Class="SampleApp.Nested" /></Window>)");

    const auto result = ResolveXaml(roots, "MainWindow.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("x:Class is only valid on the XAML root"), std::string::npos);
}

TEST(CppEmitterTests, EmitsTypedBuilderComponent)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Grid x:Name=\"RootGrid\"><Button x:Name=\"ApplyButton\" Text=\"Apply\" /></Grid>");
    const auto resolved = ResolveXaml(roots, "MainWindow.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"App", "MainWindow", "App/Generated/MainWindow.xaml.g.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    EXPECT_NE(generated.header.find("#include <NeoWPF/NeoWPF.h>"), std::string::npos);
    EXPECT_EQ(generated.header.find("#include <NeoWPF/UI/Markup/Builder/Builder.h>"), std::string::npos);
    EXPECT_NE(generated.header.find("std::shared_ptr<NeoWPF::UI::Grid> Root"), std::string::npos);
    EXPECT_NE(generated.header.find("std::shared_ptr<NeoWPF::UI::Button> ApplyButton"), std::string::npos);
    EXPECT_NE(generated.header.find("NeoWPF/UI/Layout/Grid/Grid.h"), std::string::npos);
    EXPECT_NE(generated.header.find("NeoWPF/UI/Controls/Button/Button.h"), std::string::npos);
    EXPECT_NE(generated.source.find("NeoWPF::UI::Builder::Build<NeoWPF::UI::Grid>()"), std::string::npos);
    EXPECT_NE(generated.source.find(".Name(\"ApplyButton\")"), std::string::npos);
    EXPECT_NE(generated.source.find(".Set(NeoWPF::UI::Button::TextProperty(), std::wstring(L\"Apply\"))"), std::string::npos);
    EXPECT_NE(generated.source.find(".AddChild(element1)"), std::string::npos);
    EXPECT_NE(generated.source.find("if (Root)"), std::string::npos);
}

TEST(CppEmitterTests, EmitsWindowRootSubclass)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Window x:Class="SampleApp.MainWindow" Title="Main"><Window.Content><Grid /></Window.Content></Window>)");
    const auto resolved = ResolveXaml(roots, "MainWindow.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"SampleApp", "MainWindow", "SampleApp/MainWindow.xaml.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    EXPECT_NE(generated.header.find("class MainWindow : public NeoWPF::UI::Window"), std::string::npos);
    EXPECT_NE(generated.header.find("MainWindow();"), std::string::npos);
    EXPECT_EQ(generated.header.find("std::shared_ptr<NeoWPF::UI::Window> Root"), std::string::npos);
    EXPECT_EQ(generated.header.find("m_componentInitialized"), std::string::npos);
    EXPECT_NE(generated.source.find("ElementBuilder<NeoWPF::UI::Window>(std::shared_ptr<NeoWPF::UI::Window>(this"), std::string::npos);
    EXPECT_NE(generated.source.find("if (IsXamlComponentInitialized())"), std::string::npos);
    EXPECT_NE(generated.source.find("MarkXamlComponentInitialized();"), std::string::npos);
    EXPECT_NE(generated.source.find("Window::ContentProperty()"), std::string::npos);
}

TEST(CppEmitterTests, EmitsApplicationRootSubclass)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<Application x:Class="SampleApp.App" />)");
    const auto resolved = ResolveXaml(roots, "App.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"SampleApp", "App", "SampleApp/App.xaml.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    EXPECT_NE(generated.header.find("class App : public NeoWPF::Core::Application"), std::string::npos);
    EXPECT_NE(generated.header.find("App();"), std::string::npos);
    EXPECT_EQ(generated.header.find("std::shared_ptr<NeoWPF::Core::Application> Root"), std::string::npos);
    EXPECT_NE(generated.source.find("ElementBuilder<NeoWPF::Core::Application>(std::shared_ptr<NeoWPF::Core::Application>(this"), std::string::npos);
    EXPECT_EQ(generated.header.find("m_componentInitialized"), std::string::npos);
    EXPECT_NE(generated.source.find("if (IsXamlComponentInitialized())"), std::string::npos);
}

TEST(CppEmitterTests, EmitsAttachedPropertyOwnerAccessor)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Grid><Button Grid.Row=\"2\" /></Grid>");
    const auto resolved = ResolveXaml(roots, "Attached.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"App", "Attached", "App/Generated/Attached.xaml.g.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    EXPECT_NE(generated.source.find(".Set(NeoWPF::UI::Grid::RowProperty(), 2)"), std::string::npos);
}

TEST(CppEmitterTests, EmitsPropertyElementAssignment)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Grid><Grid.RowDefinitions><RowDefinition /></Grid.RowDefinitions></Grid>");
    const auto resolved = ResolveXaml(roots, "Definitions.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"App", "Definitions", "App/Generated/Definitions.xaml.g.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    EXPECT_NE(generated.source.find(".Assign(NeoWPF::UI::Grid::RowDefinitionsProperty(), element1)"), std::string::npos);
}

TEST(CppEmitterTests, EmitsBindingOperations)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Grid><TextBlock Text=\"{Binding Title, Mode=TwoWay}\" /></Grid>");
    const auto resolved = ResolveXaml(roots, "Binding.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"App", "Binding", "App/Generated/Binding.xaml.g.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    EXPECT_NE(generated.source.find(
        "BindingOperations::SetBinding(*element1, NeoWPF::UI::TextBlock::TextProperty(), NeoWPF::UI::Data::Binding(\"Title\", NeoWPF::UI::Data::BindingMode::TwoWay))"),
        std::string::npos);
}

TEST(CppEmitterTests, EmitsLocaleOperations)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Grid><TextBlock Text=\"{Locale Greeting}\" /></Grid>");
    const auto resolved = ResolveXaml(roots, "Locale.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"App", "Locale", "App/Generated/Locale.xaml.g.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    EXPECT_NE(generated.source.find(
        "LocaleOperations::SetLocale(*element1, NeoWPF::UI::TextBlock::TextProperty(), NeoWPF::Platform::Utf8ToWide(\"Greeting\"))"),
        std::string::npos);
}

TEST(CppEmitterTests, EmitsResourceRegistrationBeforeStaticResourceAssignment)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Border><Border.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" Text=\"Compiled\" /></ResourceDictionary></Border.Resources><Border.Content><Border Content=\"{StaticResource Accent}\" /></Border.Content></Border>");
    const auto resolved = ResolveXaml(roots, "GeneratedResources.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"App", "Resource", "App/Generated/Resource.xaml.g.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    const auto resourceRegistration = generated.source.find("SetResource(\"Accent\"");
    const auto resourceAssignment = generated.source.find(".Assign(NeoWPF::UI::Content::ContentControl::ContentProperty(), element");
    ASSERT_NE(resourceRegistration, std::string::npos);
    ASSERT_NE(resourceAssignment, std::string::npos) << generated.source;
    EXPECT_LT(resourceRegistration, resourceAssignment);
    EXPECT_EQ(generated.source.find(".AddChild(element1)"), std::string::npos);
}

TEST(CppEmitterTests, EmitsImplicitStyleResourceRegistration)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Grid><Grid.Resources><ResourceDictionary><Style TargetType=\"Button\"><Setter Property=\"Text\" Value=\"Compiled implicit\" /></Style></ResourceDictionary></Grid.Resources><Button /></Grid>");
    const auto resolved = ResolveXaml(roots, "ImplicitStyle.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"App", "ImplicitStyle", "App/Generated/ImplicitStyle.xaml.g.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    EXPECT_NE(generated.source.find(
        ".Set(NeoWPF::UI::Styling::Style::TargetTypeProperty(), std::type_index(typeid(NeoWPF::UI::Button)))"),
        std::string::npos);
    EXPECT_NE(generated.source.find(
        "->AddSetter(element"),
        std::string::npos);
    EXPECT_NE(generated.source.find(
        "->SetImplicitXamlStyle(element"),
        std::string::npos);
}

TEST(CppEmitterTests, EmitsNearestNestedStaticResource)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        "<Border><Border.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" Text=\"Outer\" /></ResourceDictionary></Border.Resources><Border><Border.Resources><ResourceDictionary><TextBlock x:Key=\"Accent\" Text=\"Inner\" /></ResourceDictionary></Border.Resources><Border.Content><TextBlock Text=\"{StaticResource Accent}\" /></Border.Content></Border></Border>");
    const auto resolved = ResolveXaml(roots, "NestedResources.xaml");
    ASSERT_TRUE(resolved.success) << resolved.error;

    const auto generated = EmitCppComponent(
        resolved.document,
        {"App", "NestedResources", "App/Generated/NestedResources.xaml.g.h"});

    ASSERT_TRUE(generated.success) << generated.error;
    EXPECT_NE(generated.source.find(
        "builder2.Assign(NeoWPF::UI::TextBlock::TextProperty(), element3)"),
        std::string::npos) << generated.source;
    EXPECT_EQ(generated.source.find(
        "builder2.Assign(NeoWPF::UI::TextBlock::TextProperty(), element4)"),
        std::string::npos);
}

TEST(NamlCompilerTests, RejectsEmptySource)
{
    const auto result = CompileXaml("", "Empty.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("Empty.xaml"), std::string::npos);
}

TEST(NamlCompilerTests, RejectsUnknownType)
{
    const auto result = CompileXaml("<NotAControl />", "UnknownType.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("unknown XAML type 'NotAControl'"), std::string::npos);
    EXPECT_NE(result.error.find("1:2"), std::string::npos);
}

TEST(NamlCompilerTests, RejectsUnknownProperty)
{
    const auto result = CompileXaml("<Button DoesNotExist=\"value\" />", "UnknownProperty.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("unknown property 'DoesNotExist'"), std::string::npos);
}

TEST(NamlCompilerTests, RejectsInvalidPropertyValue)
{
    const auto result = CompileXaml("<Grid Padding=\"not thickness\" />", "InvalidValue.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("invalid value for property 'Padding'"), std::string::npos);
}

TEST(NamlCompilerTests, AcceptsTextAlignmentAndFontWeightProperties)
{
    const auto result = CompileXaml(
        "<Grid><TextBlock TextHorizontalAlignment=\"Center\" FontWeight=\"Bold\" /><Button TextHorizontalAlignment=\"Right\" TextVerticalAlignment=\"Center\" FontWeight=\"700\" /></Grid>",
        "TextProperties.xaml");

    EXPECT_TRUE(result.success) << result.error;
}

TEST(NamlCompilerTests, ResolvesInheritedControlProperties)
{
    const auto result = CompileXaml(
        "<Button Background=\"#FF101820\" BorderBrush=\"#FFE05A3F\" BorderThickness=\"1\" />",
        "ControlProperties.xaml");

    EXPECT_TRUE(result.success) << result.error;
}

TEST(NamlCompilerTests, RejectsDuplicateNames)
{
    const auto result = CompileXaml(
        "<Grid><Button x:Name=\"Action\" /><TextBlock x:Name=\"Action\" /></Grid>",
        "DuplicateName.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("duplicate XAML name 'Action'"), std::string::npos);
}

}
}
