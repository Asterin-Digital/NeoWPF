#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/Resources/EmbeddedResourceProvider.h>
#include <NeoWPF/Core/Application/Resources/ResourceManager.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Markup/Xaml/Loading/CompiledXaml.h>
#include <NeoNaml/NamlArtifact.h>

#include <string>

namespace {

TEST(NamlArtifactTests, RoundTripsArtifactThroughSharedImplementation)
{
    NeoNaml::NamlArtifact expected;
    expected.resourceKey = "MainWindow\"Resource";
    expected.sourcePath = "views\\MainWindow\n.xaml";
    expected.rootObjectId = 0;
    expected.strings = {"Grid", "Children", "Text"};
    expected.objects = {
        {0, 0, "Root"},
        {1, 1, "ApplyButton"}
    };
    expected.properties = {
        {1, 2, {NeoNaml::NamlValueKind::String, "Apply\tNow"}},
        {1, 2, {NeoNaml::NamlValueKind::Integer, "42"}},
        {1, 2, {NeoNaml::NamlValueKind::FloatingPoint, "3.5"}},
        {1, 2, {NeoNaml::NamlValueKind::Boolean, "true"}},
        {1, 2, {NeoNaml::NamlValueKind::Null, ""}},
        {1, 2, {NeoNaml::NamlValueKind::MarkupExtension, "{StaticResource Accent}"}}
    };
    expected.children = {{0, 1, 1}};
    expected.bindings = {{1, 2, "Query.Path", "TwoWay"}};

    const auto serialized = NeoNaml::SerializeNaml(expected);
    NeoNaml::NamlArtifact actual;
    std::string error;

    ASSERT_TRUE(NeoNaml::DeserializeNaml(
        reinterpret_cast<const unsigned char*>(serialized.data()),
        serialized.size(),
        actual,
        error)) << error;

    EXPECT_EQ(actual.resourceKey, expected.resourceKey);
    EXPECT_EQ(actual.sourcePath, expected.sourcePath);
    EXPECT_EQ(actual.rootObjectId, expected.rootObjectId);
    EXPECT_EQ(actual.strings, expected.strings);
    EXPECT_EQ(actual.objects.size(), expected.objects.size());
    ASSERT_EQ(actual.objects.size(), 2u);
    EXPECT_EQ(actual.objects[0].name, "Root");
    EXPECT_EQ(actual.objects[1].name, "ApplyButton");
    EXPECT_EQ(actual.objects[0].resourceKey, "");
    EXPECT_EQ(actual.objects[1].resourceKey, "");
    EXPECT_EQ(actual.properties.size(), expected.properties.size());
    ASSERT_EQ(actual.properties.size(), 6u);
    for (std::size_t index = 0; index < expected.properties.size(); ++index) {
        EXPECT_EQ(actual.properties[index].value.kind, expected.properties[index].value.kind);
        EXPECT_EQ(actual.properties[index].value.text, expected.properties[index].value.text);
    }
    ASSERT_EQ(actual.children.size(), 1u);
    EXPECT_EQ(actual.children[0].parentId, expected.children[0].parentId);
    EXPECT_EQ(actual.children[0].childId, expected.children[0].childId);
    EXPECT_EQ(actual.children[0].propertyString, expected.children[0].propertyString);
    ASSERT_EQ(actual.bindings.size(), 1u);
    EXPECT_EQ(actual.bindings[0].objectId, 1u);
    EXPECT_EQ(actual.bindings[0].propertyString, 2u);
    EXPECT_EQ(actual.bindings[0].path, "Query.Path");
    EXPECT_EQ(actual.bindings[0].mode, "TwoWay");
}

TEST(NamlArtifactTests, RejectsUnsupportedValueKind)
{
    const std::string input =
        "NAML1\n"
        "version=1\n"
        "resourceKey=\"Test\"\n"
        "source=\"Test.xaml\"\n"
        "root=0\n\n"
        "objects {\n"
        "  0: type=0\n"
        "}\n\n"
        "properties {\n"
        "  object=0 property=0 kind=unknown value=\"value\"\n"
        "}\n\n"
        "end\n";

    NeoNaml::NamlArtifact artifact;
    std::string error;

    EXPECT_FALSE(NeoNaml::DeserializeNaml(
        reinterpret_cast<const unsigned char*>(input.data()),
        input.size(),
        artifact,
        error));
    EXPECT_EQ(error, "malformed NAML property record");
}

TEST(NamlArtifactTests, LoadsImplicitContentThroughRegisteredContentProperty)
{
    NeoWPF::Core::Application::Initialize();

    NeoNaml::NamlArtifact artifact;
    artifact.resourceKey = "CompiledContentTest";
    artifact.sourcePath = "CompiledContentTest.xaml";
    artifact.rootObjectId = 0;
    artifact.strings = {"Grid", "Border", "TextBlock", "Children", "Text"};
    artifact.objects = {
        {0, 0, "Root"},
        {1, 1, "ContentBorder"},
        {2, 2, "ContentText"}
    };
    artifact.properties = {{2, 4, {NeoNaml::NamlValueKind::String, "Compiled content"}}};
    artifact.children = {{0, 1, 3}, {1, 2, 3}};

    const auto serialized = NeoNaml::SerializeNaml(artifact);
    const NeoWPF::ResourceKey key("CompiledContentTest");
    NeoWPF::ResourceManager::Instance().RegisterProviderReference(
        NeoWPF::EmbeddedResourceProvider::Instance());
    NeoWPF::EmbeddedResourceProvider::Instance().Register(
        key,
        NeoWPF::ResourceType::Xaml,
        reinterpret_cast<const unsigned char*>(serialized.data()),
        serialized.size());

    std::string error;
    NeoWPF::UI::Markup::Xaml::XamlNamescope namescope;
    const auto root = NeoWPF::UI::Markup::Xaml::LoadCompiledXaml(key, &namescope, &error);

    ASSERT_NE(root, nullptr) << error;
    const auto grid = std::dynamic_pointer_cast<NeoWPF::UI::Grid>(root);
    ASSERT_NE(grid, nullptr);
    const auto children = grid->GetLogicalChildren();
    ASSERT_EQ(children.size(), 1u);
    const auto border = dynamic_cast<NeoWPF::UI::Border*>(children[0]);
    ASSERT_NE(border, nullptr);
    const auto text = dynamic_cast<NeoWPF::UI::TextBlock*>(border->GetChild().get());
    ASSERT_NE(text, nullptr);
    EXPECT_EQ(text->GetText(), L"Compiled content");
    EXPECT_EQ(namescope.Find<NeoWPF::UI::Grid>("Root").get(), root.get());
    EXPECT_EQ(namescope.Find<NeoWPF::UI::Border>("ContentBorder").get(), border);
    EXPECT_EQ(namescope.Find<NeoWPF::UI::TextBlock>("ContentText").get(), text);

    NeoWPF::EmbeddedResourceProvider::Instance().Unregister(key);
    NeoWPF::Core::Application::Shutdown();
}

TEST(NamlArtifactTests, LoadsKeyedResourcesWithoutAddingThemToVisualChildren)
{
    NeoWPF::Core::Application::Initialize();

    NeoNaml::NamlArtifact artifact;
    artifact.resourceKey = "CompiledResourceTest";
    artifact.sourcePath = "CompiledResourceTest.xaml";
    artifact.rootObjectId = 0;
    artifact.strings = {"Grid", "TextBlock", "Resources", "Text"};
    artifact.objects = {
        {0, 0, "Root", ""},
        {1, 1, "", "Accent"}
    };
    artifact.properties = {{1, 3, {NeoNaml::NamlValueKind::String, "Resource text"}}};
    artifact.children = {{0, 1, 2}};

    const auto key = NeoWPF::ResourceKey("CompiledResourceTest");
    const auto serialized = NeoNaml::SerializeNaml(artifact);
    NeoWPF::ResourceManager::Instance().RegisterProviderReference(
        NeoWPF::EmbeddedResourceProvider::Instance());
    NeoWPF::EmbeddedResourceProvider::Instance().Register(
        key,
        NeoWPF::ResourceType::Xaml,
        reinterpret_cast<const unsigned char*>(serialized.data()),
        serialized.size());

    std::string error;
    const auto root = NeoWPF::UI::Markup::Xaml::LoadCompiledXaml(key, &error);
    ASSERT_NE(root, nullptr) << error;
    ASSERT_EQ(root->GetVisualChildren().size(), 0u);

    const auto element = dynamic_cast<NeoWPF::UI::UIElement*>(root.get());
    ASSERT_NE(element, nullptr);
    std::any resource;
    ASSERT_TRUE(element->TryFindResource("Accent", resource));
    const auto resourceText = std::any_cast<std::shared_ptr<NeoWPF::UI::UIElement>>(resource);
    ASSERT_NE(resourceText, nullptr);
    EXPECT_EQ(std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(resourceText)->GetText(), L"Resource text");

    NeoWPF::EmbeddedResourceProvider::Instance().Unregister(key);
    NeoWPF::Core::Application::Shutdown();
}

TEST(NamlArtifactTests, ResolvesStaticResourceIntoContentProperty)
{
    NeoWPF::Core::Application::Initialize();

    NeoNaml::NamlArtifact artifact;
    artifact.resourceKey = "CompiledStaticResourceTest";
    artifact.sourcePath = "CompiledStaticResourceTest.xaml";
    artifact.rootObjectId = 0;
    artifact.strings = {"Border", "TextBlock", "Resources", "Text", "Content"};
    artifact.objects = {
        {0, 0, "", ""},
        {1, 1, "", "Accent"}
    };
    artifact.properties = {
        {1, 3, {NeoNaml::NamlValueKind::String, "Resource text"}},
        {0, 4, {NeoNaml::NamlValueKind::MarkupExtension, "{StaticResource Accent}"}}
    };
    artifact.children = {{0, 1, 2}};

    const auto key = NeoWPF::ResourceKey("CompiledStaticResourceTest");
    const auto serialized = NeoNaml::SerializeNaml(artifact);
    NeoWPF::ResourceManager::Instance().RegisterProviderReference(
        NeoWPF::EmbeddedResourceProvider::Instance());
    NeoWPF::EmbeddedResourceProvider::Instance().Register(
        key,
        NeoWPF::ResourceType::Xaml,
        reinterpret_cast<const unsigned char*>(serialized.data()),
        serialized.size());

    std::string error;
    const auto root = NeoWPF::UI::Markup::Xaml::LoadCompiledXaml(key, &error);
    ASSERT_NE(root, nullptr) << error;
    const auto border = std::dynamic_pointer_cast<NeoWPF::UI::Border>(root);
    ASSERT_NE(border, nullptr);
    ASSERT_NE(border->GetChild(), nullptr);
    const auto contentText = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(border->GetChild());
    ASSERT_NE(contentText, nullptr);
    EXPECT_EQ(contentText->GetText(), L"Resource text");

    NeoWPF::EmbeddedResourceProvider::Instance().Unregister(key);
    NeoWPF::Core::Application::Shutdown();
}

TEST(NamlArtifactTests, RejectsCompiledResourceWithoutKey)
{
    NeoWPF::Core::Application::Initialize();

    NeoNaml::NamlArtifact artifact;
    artifact.resourceKey = "MissingCompiledResourceKey";
    artifact.sourcePath = "MissingCompiledResourceKey.xaml";
    artifact.rootObjectId = 0;
    artifact.strings = {"Grid", "TextBlock", "Resources"};
    artifact.objects = {{0, 0, "", ""}, {1, 1, "", ""}};
    artifact.children = {{0, 1, 2}};

    const auto key = NeoWPF::ResourceKey("MissingCompiledResourceKey");
    const auto serialized = NeoNaml::SerializeNaml(artifact);
    NeoWPF::ResourceManager::Instance().RegisterProviderReference(
        NeoWPF::EmbeddedResourceProvider::Instance());
    NeoWPF::EmbeddedResourceProvider::Instance().Register(
        key, NeoWPF::ResourceType::Xaml,
        reinterpret_cast<const unsigned char*>(serialized.data()), serialized.size());

    std::string error;
    EXPECT_EQ(NeoWPF::UI::Markup::Xaml::LoadCompiledXaml(key, &error), nullptr);
    EXPECT_NE(error.find("missing a key"), std::string::npos);

    NeoWPF::EmbeddedResourceProvider::Instance().Unregister(key);
    NeoWPF::Core::Application::Shutdown();
}

TEST(NamlArtifactTests, RejectsMissingCompiledStaticResource)
{
    NeoWPF::Core::Application::Initialize();

    NeoNaml::NamlArtifact artifact;
    artifact.resourceKey = "MissingCompiledStaticResource";
    artifact.sourcePath = "MissingCompiledStaticResource.xaml";
    artifact.rootObjectId = 0;
    artifact.strings = {"TextBlock", "Text"};
    artifact.objects = {{0, 0, "", ""}};
    artifact.properties = {{0, 1, {NeoNaml::NamlValueKind::MarkupExtension, "{StaticResource Missing}"}}};

    const auto key = NeoWPF::ResourceKey("MissingCompiledStaticResource");
    const auto serialized = NeoNaml::SerializeNaml(artifact);
    NeoWPF::ResourceManager::Instance().RegisterProviderReference(
        NeoWPF::EmbeddedResourceProvider::Instance());
    NeoWPF::EmbeddedResourceProvider::Instance().Register(
        key, NeoWPF::ResourceType::Xaml,
        reinterpret_cast<const unsigned char*>(serialized.data()), serialized.size());

    std::string error;
    EXPECT_EQ(NeoWPF::UI::Markup::Xaml::LoadCompiledXaml(key, &error), nullptr);
    EXPECT_NE(error.find("StaticResource was not found"), std::string::npos);

    NeoWPF::EmbeddedResourceProvider::Instance().Unregister(key);
    NeoWPF::Core::Application::Shutdown();
}

TEST(NamlArtifactTests, ResolvesResourceFromAncestorScope)
{
    NeoWPF::Core::Application::Initialize();

    NeoNaml::NamlArtifact artifact;
    artifact.resourceKey = "CompiledAncestorResource";
    artifact.sourcePath = "CompiledAncestorResource.xaml";
    artifact.rootObjectId = 0;
    artifact.strings = {"Grid", "TextBlock", "Resources", "Text", "Border", "Content", "Children"};
    artifact.objects = {
        {0, 0, "", ""},
        {1, 1, "", "Accent"},
        {2, 4, "", ""}
    };
    artifact.properties = {
        {1, 3, {NeoNaml::NamlValueKind::String, "Ancestor resource"}},
        {2, 5, {NeoNaml::NamlValueKind::MarkupExtension, "{StaticResource Accent}"}}
    };
    artifact.children = {{0, 1, 2}, {0, 2, 6}};

    const auto key = NeoWPF::ResourceKey("CompiledAncestorResource");
    const auto serialized = NeoNaml::SerializeNaml(artifact);
    NeoWPF::ResourceManager::Instance().RegisterProviderReference(
        NeoWPF::EmbeddedResourceProvider::Instance());
    NeoWPF::EmbeddedResourceProvider::Instance().Register(
        key, NeoWPF::ResourceType::Xaml,
        reinterpret_cast<const unsigned char*>(serialized.data()), serialized.size());

    std::string error;
    const auto root = NeoWPF::UI::Markup::Xaml::LoadCompiledXaml(key, &error);
    ASSERT_NE(root, nullptr) << error;
    const auto grid = std::dynamic_pointer_cast<NeoWPF::UI::Grid>(root);
    ASSERT_NE(grid, nullptr);
    ASSERT_EQ(grid->GetVisualChildren().size(), 1u);
    const auto border = dynamic_cast<NeoWPF::UI::Border*>(grid->GetVisualChildren()[0]);
    ASSERT_NE(border, nullptr);
    const auto text = dynamic_cast<NeoWPF::UI::TextBlock*>(border->GetChild().get());
    ASSERT_NE(text, nullptr);
    EXPECT_EQ(text->GetText(), L"Ancestor resource");

    NeoWPF::EmbeddedResourceProvider::Instance().Unregister(key);
    NeoWPF::Core::Application::Shutdown();
}

TEST(NamlArtifactTests, RejectsDuplicateKeysInOneCompiledScope)
{
    NeoWPF::Core::Application::Initialize();

    NeoNaml::NamlArtifact artifact;
    artifact.resourceKey = "DuplicateCompiledResource";
    artifact.sourcePath = "DuplicateCompiledResource.xaml";
    artifact.rootObjectId = 0;
    artifact.strings = {"Grid", "TextBlock", "Resources", "Text"};
    artifact.objects = {
        {0, 0, "", ""},
        {1, 1, "", "Accent"},
        {2, 1, "", "Accent"}
    };
    artifact.properties = {
        {1, 3, {NeoNaml::NamlValueKind::String, "First"}},
        {2, 3, {NeoNaml::NamlValueKind::String, "Second"}}
    };
    artifact.children = {{0, 1, 2}, {0, 2, 2}};

    const auto key = NeoWPF::ResourceKey("DuplicateCompiledResource");
    const auto serialized = NeoNaml::SerializeNaml(artifact);
    NeoWPF::ResourceManager::Instance().RegisterProviderReference(
        NeoWPF::EmbeddedResourceProvider::Instance());
    NeoWPF::EmbeddedResourceProvider::Instance().Register(
        key, NeoWPF::ResourceType::Xaml,
        reinterpret_cast<const unsigned char*>(serialized.data()), serialized.size());

    std::string error;
    EXPECT_EQ(NeoWPF::UI::Markup::Xaml::LoadCompiledXaml(key, &error), nullptr);
    EXPECT_NE(error.find("duplicate NAML resource key"), std::string::npos);

    NeoWPF::EmbeddedResourceProvider::Instance().Unregister(key);
    NeoWPF::Core::Application::Shutdown();
}

}
