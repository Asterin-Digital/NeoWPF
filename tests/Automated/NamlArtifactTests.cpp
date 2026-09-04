#include <gtest/gtest.h>

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

}
