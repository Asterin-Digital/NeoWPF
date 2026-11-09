#include <NeoPM/Contracts/IProjectType.h>
#include <NeoPM/Core/Project/CMakeListsBuilder.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <algorithm>
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

TEST(ProjectTypeTests, MapsPlatformPresetsToTheirPinnedSdkInstallations) {
    TemporaryProjectDirectory directory;
    NeoPM::Core::Project::CMakeListsBuilder builder("SampleApp");
    const auto windowsRoot = directory.Path() / "SDK" / "Windows-x64" / "1.4.0";
    const auto linuxRoot = directory.Path() / "SDK" / "Linux-x64" / "1.3.2";
    std::map<std::string, NeoPM::Contracts::SdkPlatformConfiguration> sdkPlatforms{
        {"Windows", {
            "1.4.0",
            {windowsRoot / "include", windowsRoot / "third_party" / "include"},
            windowsRoot / "Debug" / "NeoWPF.Windows.Debug.lib",
            windowsRoot / "Release" / "NeoWPF.Windows.Release.lib"}},
        {"Linux", {
            "1.3.2",
            {linuxRoot / "include", linuxRoot / "third_party" / "include"},
            linuxRoot / "Debug" / "libNeoWPF.Linux.Debug.a",
            linuxRoot / "Release" / "libNeoWPF.Linux.Release.a"}}};
    auto context = MakeContext(directory.Path());
    context.defaultConfiguration = "release";
    context.sdkPlatforms = sdkPlatforms;
    builder.SetSdkPlatforms(sdkPlatforms);

    ASSERT_TRUE(builder.Generate(context));

    std::ifstream cmakeInput(directory.Path() / "CMakeLists.txt");
    const std::string cmakeContent{
        std::istreambuf_iterator<char>(cmakeInput),
        std::istreambuf_iterator<char>()};
    std::ifstream presetsInput(directory.Path() / "CMakePresets.json");
    const nlohmann::json presets = nlohmann::json::parse(std::string{
        std::istreambuf_iterator<char>(presetsInput),
        std::istreambuf_iterator<char>()});

    EXPECT_NE(cmakeContent.find("NEOWPF_INCLUDE_DIRECTORIES"), std::string::npos);
    EXPECT_NE(cmakeContent.find("Windows-x64/1.4.0/third_party/include"), std::string::npos);
    EXPECT_NE(cmakeContent.find("Linux-x64/1.3.2/third_party/include"), std::string::npos);
    EXPECT_NE(cmakeContent.find("Windows-x64/1.4.0/Debug/NeoWPF.Windows.Debug.lib"), std::string::npos);
    EXPECT_NE(cmakeContent.find("Linux-x64/1.3.2/Release/libNeoWPF.Linux.Release.a"), std::string::npos);
    const auto findPreset = [&presets](const std::string& name) {
        return std::find_if(presets["configurePresets"].begin(), presets["configurePresets"].end(),
            [&name](const auto& preset) { return preset["name"] == name; });
    };
    const auto windowsPreset = findPreset("windows-base");
    const auto linuxPreset = findPreset("linux-base");
    ASSERT_NE(windowsPreset, presets["configurePresets"].end());
    ASSERT_NE(linuxPreset, presets["configurePresets"].end());
    EXPECT_EQ((*windowsPreset)["cacheVariables"]["NEOWPF_SDK_VERSION"], "1.4.0");
    EXPECT_EQ((*linuxPreset)["cacheVariables"]["NEOWPF_SDK_VERSION"], "1.3.2");
    EXPECT_EQ((*windowsPreset)["cacheVariables"]["NEOWPF_LIBRARY_DEBUG"],
        (windowsRoot / "Debug" / "NeoWPF.Windows.Debug.lib").generic_string());
    EXPECT_EQ((*windowsPreset)["cacheVariables"]["NEOWPF_LIBRARY_RELEASE"],
        (windowsRoot / "Release" / "NeoWPF.Windows.Release.lib").generic_string());
    EXPECT_EQ((*linuxPreset)["cacheVariables"]["NEOWPF_LIBRARY_DEBUG"],
        (linuxRoot / "Debug" / "libNeoWPF.Linux.Debug.a").generic_string());
    EXPECT_EQ((*linuxPreset)["cacheVariables"]["NEOWPF_LIBRARY_RELEASE"],
        (linuxRoot / "Release" / "libNeoWPF.Linux.Release.a").generic_string());
    EXPECT_EQ(cmakeContent.find("NEOWPF_ROOT"), std::string::npos);
}
