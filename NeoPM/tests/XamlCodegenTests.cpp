#include <NeoPM/Resources/ResourceManifest.h>
#include <NeoPM/Resources/XamlCodegen.h>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

class TemporaryProjectDirectory {
public:
    TemporaryProjectDirectory()
    {
        const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
            ("NeoPM.XamlCodegenTests." + std::to_string(suffix));
        std::filesystem::create_directories(m_path);
    }

    ~TemporaryProjectDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(m_path, error);
    }

    const std::filesystem::path& Path() const noexcept
    {
        return m_path;
    }

private:
    std::filesystem::path m_path;
};

void WriteText(const std::filesystem::path& path, const std::string& content)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(output.is_open());
    output << content;
}

std::string ReadText(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

}

TEST(XamlCodegenTests, GeneratesNamlAndTypedComponentFiles)
{
    TemporaryProjectDirectory project;
    const auto root = project.Path();
    WriteText(root / "Resources" / "resources.json",
        "{\n"
        "  \"resources\": [],\n"
        "  \"xaml\": [\n"
        "    {\"key\": \"App\", \"path\": \"App.xaml\"},\n"
        "    {\"key\": \"MainWindow\", \"path\": \"Views/MainWindow.xaml\"},\n"
        "    {\"key\": \"Content\", \"path\": \"Views/Content.xaml\"}\n"
        "  ]\n"
        "}\n");
    WriteText(root / "App.xaml",
        "<Application x:Class=\"SampleApp.App\" />\n");
    WriteText(root / "Views" / "MainWindow.xaml",
        "<Window x:Class=\"SampleApp.MainWindow\" Title=\"Main\"><Window.Content><Grid><Button x:Name=\"ApplyButton\" Text=\"Apply\" /></Grid></Window.Content></Window>\n");
    WriteText(root / "Views" / "Content.xaml",
        "<Grid />\n");

    const auto manifest = NeoPM::Core::Resources::ResourceManifest::Load(
        root / "Resources" / "resources.json");
    ASSERT_TRUE(manifest.has_value());

    const auto generatedHeader = root / "include" / "SampleApp" /
        "Views/MainWindow.xaml.h";
    const auto appHeader = root / "include" / "SampleApp" / "App.xaml.h";
    const auto appSource = root / "Generated" / "App.xaml.g.cpp";
    const auto windowSource = root / "Generated" / "Views/MainWindow.xaml.g.cpp";
    const auto contentHeader = root / "include" / "SampleApp" / "Views/Content.xaml.h";
    const auto umbrellaHeader = root / "include" / "SampleApp" / "Xaml.h";
    const auto generatedSource = root / "Generated" / "Views/MainWindow.xaml.g.cpp";
    const auto artifact = root / "Generated" / ".naml" / "Views/MainWindow.xaml.g.naml";
    const auto appArtifact = root / "Generated" / ".naml" / "App.xaml.g.naml";
    WriteText(generatedHeader, "stale header");
    WriteText(generatedSource, "stale source");
    WriteText(artifact, "stale artifact");

    ASSERT_TRUE(NeoPM::Core::Resources::XamlCodegen{}.Generate(
        *manifest, root, "SampleApp"));

    EXPECT_TRUE(std::filesystem::is_regular_file(generatedHeader));
    EXPECT_TRUE(std::filesystem::is_regular_file(appHeader));
    EXPECT_TRUE(std::filesystem::is_regular_file(contentHeader));
    EXPECT_TRUE(std::filesystem::is_regular_file(umbrellaHeader));
    EXPECT_TRUE(std::filesystem::is_regular_file(appSource));
    EXPECT_TRUE(std::filesystem::is_regular_file(windowSource));
    EXPECT_TRUE(std::filesystem::is_regular_file(generatedSource));
    EXPECT_TRUE(std::filesystem::is_regular_file(artifact));
    EXPECT_TRUE(std::filesystem::is_regular_file(appArtifact));
    EXPECT_NE(ReadText(generatedHeader).find("class MainWindow : public NeoWPF::UI::Window"), std::string::npos);
    EXPECT_NE(ReadText(appHeader).find("class App : public NeoWPF::Core::Application"), std::string::npos);
    EXPECT_NE(ReadText(generatedHeader).find("class MainWindow : public NeoWPF::UI::Window"), std::string::npos);
    EXPECT_NE(ReadText(umbrellaHeader).find("SampleApp/Views/MainWindow.xaml.h"), std::string::npos);
    EXPECT_NE(ReadText(umbrellaHeader).find("SampleApp/App.xaml.h"), std::string::npos);
    EXPECT_NE(ReadText(umbrellaHeader).find("SampleApp/Views/Content.xaml.h"), std::string::npos);
    EXPECT_NE(ReadText(generatedSource).find("Button::TextProperty()"), std::string::npos);
    EXPECT_NE(ReadText(windowSource).find("MainWindow::InitializeComponent()"), std::string::npos);
    EXPECT_NE(ReadText(artifact).find("NAML1"), std::string::npos);
    EXPECT_NE(ReadText(appArtifact).find("NAML1"), std::string::npos);
}

TEST(XamlCodegenTests, RemovesStaleXamlGeneratedFiles)
{
    TemporaryProjectDirectory project;
    const auto root = project.Path();
    WriteText(root / "Resources" / "resources.json",
        "{\n"
        "  \"resources\": [],\n"
        "  \"xaml\": []\n"
        "}\n");
    const auto staleHeader = root / "include" / "SampleApp" / "Old.xaml.h";
    const auto staleSource = root / "Generated" / "Old.xaml.g.cpp";
    const auto staleArtifact = root / "Generated" / ".naml" / "Old.xaml.g.naml";
    WriteText(staleHeader, "stale");
    WriteText(staleSource, "stale");
    WriteText(staleArtifact, "stale");

    const auto manifest = NeoPM::Core::Resources::ResourceManifest::Load(
        root / "Resources" / "resources.json");
    ASSERT_TRUE(manifest.has_value());
    ASSERT_TRUE(NeoPM::Core::Resources::XamlCodegen{}.Generate(
        *manifest, root, "SampleApp"));

    EXPECT_FALSE(std::filesystem::exists(staleHeader));
    EXPECT_FALSE(std::filesystem::exists(staleSource));
    EXPECT_FALSE(std::filesystem::exists(staleArtifact));
}

TEST(XamlCodegenTests, TracksAddedAndRemovedXamlInputs)
{
    TemporaryProjectDirectory project;
    const auto root = project.Path();
    const auto manifestPath = root / "Resources" / "resources.json";
    WriteText(root / "MainWindow.xaml", "<Grid />\n");
    WriteText(manifestPath,
        "{\n"
        "  \"resources\": [],\n"
        "  \"xaml\": [{\"key\": \"MainWindow\", \"path\": \"MainWindow.xaml\"}]\n"
        "}\n");

    auto manifest = NeoPM::Core::Resources::ResourceManifest::Load(manifestPath);
    ASSERT_TRUE(manifest.has_value());
    ASSERT_TRUE(NeoPM::Core::Resources::XamlCodegen{}.Generate(
        *manifest, root, "SampleApp"));

    const auto mainHeader = root / "include" / "SampleApp" / "MainWindow.xaml.h";
    const auto mainSource = root / "Generated" / "MainWindow.xaml.g.cpp";
    const auto mainArtifact = root / "Generated" / ".naml" / "MainWindow.xaml.g.naml";
    const auto addedHeader = root / "include" / "SampleApp" / "Added.xaml.h";
    const auto addedSource = root / "Generated" / "Added.xaml.g.cpp";
    const auto addedArtifact = root / "Generated" / ".naml" / "Added.xaml.g.naml";
    const auto umbrellaHeader = root / "include" / "SampleApp" / "Xaml.h";

    EXPECT_TRUE(std::filesystem::exists(mainHeader));
    EXPECT_TRUE(std::filesystem::exists(mainSource));
    EXPECT_TRUE(std::filesystem::exists(mainArtifact));
    EXPECT_NE(ReadText(umbrellaHeader).find("SampleApp/MainWindow.xaml.h"), std::string::npos);

    WriteText(root / "Added.xaml", "<Grid />\n");
    WriteText(manifestPath,
        "{\n"
        "  \"resources\": [],\n"
        "  \"xaml\": [\n"
        "    {\"key\": \"MainWindow\", \"path\": \"MainWindow.xaml\"},\n"
        "    {\"key\": \"Added\", \"path\": \"Added.xaml\"}\n"
        "  ]\n"
        "}\n");
    manifest = NeoPM::Core::Resources::ResourceManifest::Load(manifestPath);
    ASSERT_TRUE(manifest.has_value());
    ASSERT_TRUE(NeoPM::Core::Resources::XamlCodegen{}.Generate(
        *manifest, root, "SampleApp"));

    EXPECT_TRUE(std::filesystem::exists(addedHeader));
    EXPECT_TRUE(std::filesystem::exists(addedSource));
    EXPECT_TRUE(std::filesystem::exists(addedArtifact));
    EXPECT_NE(ReadText(umbrellaHeader).find("SampleApp/Added.xaml.h"), std::string::npos);

    WriteText(manifestPath,
        "{\n"
        "  \"resources\": [],\n"
        "  \"xaml\": [{\"key\": \"Added\", \"path\": \"Added.xaml\"}]\n"
        "}\n");
    manifest = NeoPM::Core::Resources::ResourceManifest::Load(manifestPath);
    ASSERT_TRUE(manifest.has_value());
    ASSERT_TRUE(NeoPM::Core::Resources::XamlCodegen{}.Generate(
        *manifest, root, "SampleApp"));

    EXPECT_FALSE(std::filesystem::exists(mainHeader));
    EXPECT_FALSE(std::filesystem::exists(mainSource));
    EXPECT_FALSE(std::filesystem::exists(mainArtifact));
    EXPECT_EQ(ReadText(umbrellaHeader).find("SampleApp/MainWindow.xaml.h"), std::string::npos);
    EXPECT_NE(ReadText(umbrellaHeader).find("SampleApp/Added.xaml.h"), std::string::npos);
}
