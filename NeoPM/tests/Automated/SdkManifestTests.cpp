#include <NeoPM/Sdk/SdkManifest.h>
#include <NeoPM/Sdk/SdkArchiveInstaller.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

const std::string ValidManifest = R"({
    "schema_version": 1,
    "version": "1.4.0",
    "download_url": "https://example.com/sdk-1.4.0.zip",
    "shared": {
        "include_dirs": ["include", "third_party/include"],
        "license": "LICENSE"
    },
    "platforms": {
        "windows": {
            "debug": {"libraries": ["lib/windows/debug/NeoWPF.lib"]},
            "release": {"libraries": ["lib/windows/release/NeoWPF.lib"]}
        },
        "linux": {
            "debug": {"libraries": ["lib/linux/debug/libNeoWPF.a"]},
            "release": {"libraries": ["lib/linux/release/libNeoWPF.a"]}
        },
        "android": {
            "debug": {"libraries": ["lib/android/debug/libNeoWPF.a"]},
            "release": {"libraries": ["lib/android/release/libNeoWPF.a"]}
        }
    }
})";

bool RejectsManifest(const std::string& manifest, const std::string& platform)
{
    try {
        static_cast<void>(NeoPM::Core::SdkManifestParser::Parse(manifest, platform, "x64"));
    }
    catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

class TemporarySdkDirectory {
public:
    TemporarySdkDirectory()
    {
        const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
            ("NeoPM.SdkManifestTests." + std::to_string(suffix));
        std::filesystem::create_directories(m_path);
    }

    ~TemporarySdkDirectory()
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

}

TEST(SdkManifestTests, ParsesCurrentPlatformAndMultipleIncludeDirectories) {
    const auto release = NeoPM::Core::SdkManifestParser::Parse(ValidManifest, "Windows", "x64");

    ASSERT_TRUE(release);
    EXPECT_EQ(release->version, "1.4.0");
    EXPECT_EQ(release->downloadUrl, "https://example.com/sdk-1.4.0.zip");
    EXPECT_EQ(release->debugLibraryPath, "lib/windows/debug/NeoWPF.lib");
    EXPECT_EQ(release->releaseLibraryPath, "lib/windows/release/NeoWPF.lib");
    EXPECT_EQ(release->includePaths, (std::vector<std::string>{"include", "third_party/include"}));
}

TEST(SdkManifestTests, SelectsAndroidLibraryPaths) {
    const auto release = NeoPM::Core::SdkManifestParser::Parse(ValidManifest, "Android", "arm64-v8a");

    ASSERT_TRUE(release);
    EXPECT_EQ(release->debugLibraryPath, "lib/android/debug/libNeoWPF.a");
    EXPECT_EQ(release->releaseLibraryPath, "lib/android/release/libNeoWPF.a");
}

TEST(SdkManifestTests, RejectsUnsupportedSchemaVersion) {
    auto manifest = nlohmann::json::parse(ValidManifest);
    manifest["schema_version"] = 2;

    EXPECT_TRUE(RejectsManifest(manifest.dump(), "Windows"));
}

TEST(SdkManifestTests, RejectsUnsafeArchivePaths) {
    EXPECT_FALSE(NeoPM::Core::SdkManifestParser::IsSafeRelativePath("../escape"));
    EXPECT_FALSE(NeoPM::Core::SdkManifestParser::IsSafeRelativePath("C:/absolute"));
    EXPECT_FALSE(NeoPM::Core::SdkManifestParser::IsSafeRelativePath("include\\headers"));
    EXPECT_TRUE(NeoPM::Core::SdkManifestParser::IsSafeRelativePath("third_party/include"));
}

TEST(SdkManifestTests, RejectsMoreThanOneLibraryPerConfiguration) {
    auto manifest = nlohmann::json::parse(ValidManifest);
    manifest["platforms"]["windows"]["debug"]["libraries"].push_back("other.lib");

    EXPECT_TRUE(RejectsManifest(manifest.dump(), "Windows"));
}

TEST(SdkManifestTests, InstallsSelectedPathsAndPreservesIncludeRoots) {
    TemporarySdkDirectory temporaryDirectory;
    const auto archiveRoot = temporaryDirectory.Path() / "archive";
    const auto sdkRoot = temporaryDirectory.Path() / "installed";
    std::filesystem::create_directories(archiveRoot / "lib/windows/debug");
    std::filesystem::create_directories(archiveRoot / "lib/windows/release");
    std::filesystem::create_directories(archiveRoot / "include/NeoWPF");
    std::filesystem::create_directories(archiveRoot / "third_party/include/NeoNaml");
    std::ofstream(archiveRoot / "lib/windows/debug/NeoWPF.lib");
    std::ofstream(archiveRoot / "lib/windows/release/NeoWPF.lib");
    std::ofstream(archiveRoot / "include/NeoWPF/NeoWPF.h");
    std::ofstream(archiveRoot / "third_party/include/NeoNaml/NeoNaml.h");
    std::ofstream(archiveRoot / "LICENSE");

    const auto release = NeoPM::Core::SdkManifestParser::Parse(ValidManifest, "Windows", "x64");
    ASSERT_TRUE(release);
    ASSERT_TRUE(NeoPM::Core::SdkArchiveInstaller::Install(*release, archiveRoot, sdkRoot));

    EXPECT_TRUE(std::filesystem::is_regular_file(sdkRoot / "Debug/NeoWPF.lib"));
    EXPECT_TRUE(std::filesystem::is_regular_file(sdkRoot / "Release/NeoWPF.lib"));
    EXPECT_TRUE(std::filesystem::is_regular_file(sdkRoot / "LICENSE"));
    EXPECT_TRUE(std::filesystem::is_regular_file(sdkRoot / "include/NeoWPF/NeoWPF.h"));
    EXPECT_TRUE(std::filesystem::is_regular_file(sdkRoot / "third_party/include/NeoNaml/NeoNaml.h"));

    std::ifstream includePathsFile(sdkRoot / ".include_paths");
    const std::string includePaths{
        std::istreambuf_iterator<char>(includePathsFile),
        std::istreambuf_iterator<char>()};
    EXPECT_EQ(includePaths, "include\nthird_party/include\n");
}

TEST(SdkManifestTests, InstallerRejectsUnsafeLibraryPaths) {
    TemporarySdkDirectory temporaryDirectory;
    auto release = NeoPM::Core::SdkManifestParser::Parse(ValidManifest, "Windows", "x64");
    ASSERT_TRUE(release);
    release->debugLibraryPath = "../outside.lib";

    EXPECT_FALSE(NeoPM::Core::SdkArchiveInstaller::Install(
        *release,
        temporaryDirectory.Path() / "archive",
        temporaryDirectory.Path() / "installed"));
}

TEST(SdkManifestTests, InstallsPackNeoWpfCombinedRootLayout) {
    TemporarySdkDirectory temporaryDirectory;
    const auto archiveRoot = temporaryDirectory.Path() / "archive";
    const auto sdkRoot = temporaryDirectory.Path() / "installed";
    std::filesystem::create_directories(archiveRoot / "Windows/Debug");
    std::filesystem::create_directories(archiveRoot / "Windows/Release");
    std::filesystem::create_directories(archiveRoot / "NeoWPF");
    std::filesystem::create_directories(archiveRoot / "NeoNaml");
    std::ofstream(archiveRoot / "Windows/Debug/NeoWPF.Windows.Debug.0.1.0.lib");
    std::ofstream(archiveRoot / "Windows/Release/NeoWPF.Windows.Release.0.1.0.lib");
    std::ofstream(archiveRoot / "NeoWPF/Control.h");
    std::ofstream(archiveRoot / "NeoNaml/Document.h");
    std::ofstream(archiveRoot / "LICENSE");

    auto manifest = nlohmann::json::parse(ValidManifest);
    manifest["shared"]["include_dirs"] = {"."};
    manifest["platforms"]["windows"]["debug"]["libraries"] = {
        "Windows/Debug/NeoWPF.Windows.Debug.0.1.0.lib"};
    manifest["platforms"]["windows"]["release"]["libraries"] = {
        "Windows/Release/NeoWPF.Windows.Release.0.1.0.lib"};
    const auto release = NeoPM::Core::SdkManifestParser::Parse(
        manifest.dump(), "Windows", "x64");
    ASSERT_TRUE(release);
    ASSERT_TRUE(NeoPM::Core::SdkArchiveInstaller::Install(*release, archiveRoot, sdkRoot));

    EXPECT_TRUE(std::filesystem::is_regular_file(sdkRoot / "include/NeoWPF/Control.h"));
    EXPECT_TRUE(std::filesystem::is_regular_file(sdkRoot / "include/NeoNaml/Document.h"));
    EXPECT_FALSE(std::filesystem::exists(sdkRoot / "Windows"));

    std::ifstream includePathsFile(sdkRoot / ".include_paths");
    const std::string includePaths{
        std::istreambuf_iterator<char>(includePathsFile),
        std::istreambuf_iterator<char>()};
    EXPECT_EQ(includePaths, "include\n");
}