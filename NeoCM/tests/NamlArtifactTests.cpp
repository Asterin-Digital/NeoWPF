#include <gtest/gtest.h>

#include <NeoNaml/NamlArtifact.h>

namespace NeoCM::Core::Compiler {
namespace {

TEST(NamlArtifactTests, SerializesHeaderAndSections)
{
    NeoNaml::NamlArtifact artifact;
    artifact.resourceKey = "MainWindow";
    artifact.sourcePath = "MainWindow.xaml";
    artifact.rootObjectId = 0;
    artifact.strings = {"Grid", "Children"};
    artifact.objects.push_back({0, 0, "Root"});
    artifact.children.push_back({0, 0, 1});

    const auto output = NeoNaml::SerializeNaml(artifact);

    EXPECT_EQ(output,
        "NAML1\n"
        "version=1\n"
        "resourceKey=\"MainWindow\"\n"
        "source=\"MainWindow.xaml\"\n"
        "root=0\n\n"
        "strings {\n"
        "  0 = \"Grid\"\n"
        "  1 = \"Children\"\n"
        "}\n\n"
        "objects {\n"
        "  0: type=0 name=\"Root\"\n"
        "}\n\n"
        "properties {\n"
        "}\n\n"
        "children {\n"
        "  parent=0 child=0 property=1\n"
        "}\n\n"
        "bindings {\n"
        "}\n\n"
        "locale-references {\n"
        "}\n\n"
        "locale-tables {\n"
        "}\n\n"
        "locale-strings {\n"
        "}\n\n"
        "end\n");
}

TEST(NamlArtifactTests, EscapesQuotedAndControlCharacters)
{
    NeoNaml::NamlArtifact artifact;
    artifact.resourceKey = "Window\"Name";
    artifact.sourcePath = "line\nsource";

    const auto output = NeoNaml::SerializeNaml(artifact);

    EXPECT_NE(output.find("resourceKey=\"Window\\\"Name\""), std::string::npos);
    EXPECT_NE(output.find("source=\"line\\nsource\""), std::string::npos);
}

}
}
