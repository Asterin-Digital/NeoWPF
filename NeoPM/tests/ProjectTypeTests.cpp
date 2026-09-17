#include <NeoPM/Contracts/IProjectType.h>
#include <NeoPM/Core/Project/CMakeListsBuilder.h>
#include <NeoPM/Core/Project/VcxprojBuilder.h>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {
    class TemporaryProjectDirectory {
    public:
        TemporaryProjectDirectory() {
            const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
            m_path = std::filesystem::temp_directory_path() /
                ("NeoPM.ProjectTypeTests." + std::to_string(suffix));
            std::filesystem::create_directories(m_path);
        }

        ~TemporaryProjectDirectory() {
            std::error_code error;
            std::filesystem::remove_all(m_path, error);
        }

        const std::filesystem::path& Path() const noexcept {
            return m_path;
        }

    private:
        std::filesystem::path m_path;
    };

    NeoPM::Contracts::ProjectGenerationContext MakeContext(
        const std::filesystem::path& projectRoot
    ) {
        NeoPM::Contracts::ProjectGenerationContext context;
        context.projectName = "SampleApp";
        context.projectRoot = projectRoot;
        return context;
    }
}

TEST(ProjectTypeTests, CreatesValidVcxprojProject) {
    TemporaryProjectDirectory directory;
    NeoPM::Core::Project::VcxprojBuilder builder("SampleApp");
    NeoPM::Contracts::IProjectType& projectType = builder;

    builder
        .AddCompileFile("App.xaml.cpp")
        .AddCompileFile("MainWindow.xaml.cpp")
        .AddGeneratedXamlSources()
        .AddHeaderFile("include/SampleApp/Main.h")
        .AddTargetsImport("$(LocalAppData)\\NeoWPF\\NeoWPF.Resources.targets");

    const auto context = MakeContext(directory.Path());
    ASSERT_EQ(projectType.Name(), std::string("vcxproj"));
    ASSERT_TRUE(projectType.Generate(context));

    const auto projectFile = directory.Path() / "SampleApp.vcxproj";
    const auto filtersFile = directory.Path() / "SampleApp.vcxproj.filters";
    const auto solutionFile = directory.Path() / "SampleApp.slnx";
    ASSERT_TRUE(std::filesystem::is_regular_file(projectFile));
    ASSERT_TRUE(std::filesystem::is_regular_file(filtersFile));
    ASSERT_TRUE(std::filesystem::is_regular_file(solutionFile));

    std::ifstream input(projectFile);
    const std::string content{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()};
    EXPECT_NE(content.find("<ProjectGuid>{"), std::string::npos);
    EXPECT_NE(content.find("App.xaml.cpp"), std::string::npos);
    EXPECT_NE(content.find("MainWindow.xaml.cpp"), std::string::npos);
    EXPECT_NE(content.find("Generated\\**\\*.xaml.g.cpp"), std::string::npos);
}

TEST(ProjectTypeTests, CreatesValidCMakeListsProject) {
    TemporaryProjectDirectory directory;
    NeoPM::Core::Project::CMakeListsBuilder builder("SampleApp");
    NeoPM::Contracts::IProjectType& projectType = builder;

    builder
        .AddCompileFile("App.xaml.cpp")
        .AddCompileFile("MainWindow.xaml.cpp")
        .AddGeneratedXamlSources()
        .AddHeaderFile("include\\SampleApp\\Main.h")
        .AddResourceAssetGlob()
        .AddResourceSyncModule("cmake/NeoWPFResources.cmake");

    const auto context = MakeContext(directory.Path());
    ASSERT_EQ(projectType.Name(), std::string("cmakelists"));
    ASSERT_TRUE(projectType.Generate(context));

    const auto cmakeFile = directory.Path() / "CMakeLists.txt";
    const auto presetsFile = directory.Path() / "CMakePresets.json";
    ASSERT_TRUE(std::filesystem::is_regular_file(cmakeFile));
    ASSERT_TRUE(std::filesystem::is_regular_file(presetsFile));

    std::ifstream cmakeInput(cmakeFile);
    const std::string cmakeContent{
        std::istreambuf_iterator<char>(cmakeInput),
        std::istreambuf_iterator<char>()};
    EXPECT_NE(cmakeContent.find("project(\"SampleApp\" LANGUAGES CXX)"), std::string::npos);
    EXPECT_NE(cmakeContent.find("App.xaml.cpp"), std::string::npos);
    EXPECT_NE(cmakeContent.find("MainWindow.xaml.cpp"), std::string::npos);
    EXPECT_NE(cmakeContent.find("GENERATED_XAML_SOURCES"), std::string::npos);
    EXPECT_NE(cmakeContent.find("RESOURCE_ASSETS"), std::string::npos);
    EXPECT_NE(cmakeContent.find("neowpf_sync_resources(SampleApp)"), std::string::npos);
}
