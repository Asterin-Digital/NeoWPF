#include <NeoPM/Sdk/SdkLocator.h>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
    class TemporarySdkDirectory {
    public:
        TemporarySdkDirectory() {
            const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
            m_path = std::filesystem::temp_directory_path() /
                ("NeoPM.SdkLocatorTests." + std::to_string(suffix));
            std::filesystem::create_directories(m_path);
        }

        ~TemporarySdkDirectory() {
            std::error_code error;
            std::filesystem::remove_all(m_path, error);
        }

        const std::filesystem::path& Path() const noexcept {
            return m_path;
        }

    private:
        std::filesystem::path m_path;
    };

    void CreatePlatformSdk(const std::filesystem::path& sdkRoot) {
        const auto platformPath = sdkRoot / "Windows-x64";
        const auto sdkPath = platformPath / "1.4.0";
        std::filesystem::create_directories(sdkPath / "Debug");
        std::filesystem::create_directories(sdkPath / "Release");
        std::filesystem::create_directories(sdkPath / "include" / "NeoWPF");
        std::filesystem::create_directories(sdkPath / "third_party" / "include" / "NeoNaml");
        std::ofstream(sdkPath / ".include_paths") << "include\nthird_party/include\n";
        std::ofstream(platformPath / ".version") << "1.4.0";
        std::ofstream(sdkPath / "Debug" / "NeoWPF.Windows.Debug.1.0.0.lib");
        std::ofstream(sdkPath / "Release" / "NeoWPF.Windows.Release.1.0.0.lib");
    }
}

TEST(SdkLocatorTests, FindsBothConfigurationsInPlatformArchiveLayout) {
    TemporarySdkDirectory directory;
    CreatePlatformSdk(directory.Path());

    NeoPM::Core::SdkLocator locator(directory.Path(), "Windows");

    EXPECT_EQ(
        locator.FindLibrary("debug", "x64"),
        directory.Path() / "Windows-x64" / "1.4.0" / "Debug" / "NeoWPF.Windows.Debug.1.0.0.lib");
    EXPECT_EQ(
        locator.FindLibrary("release", "x64"),
        directory.Path() / "Windows-x64" / "1.4.0" / "Release" / "NeoWPF.Windows.Release.1.0.0.lib");
    EXPECT_EQ(locator.FindIncludeDirectories("x64"), (std::vector<std::filesystem::path>{
        directory.Path() / "Windows-x64" / "1.4.0" / "include",
        directory.Path() / "Windows-x64" / "1.4.0" / "third_party" / "include"}));
    EXPECT_TRUE(locator.HasAnySdk());
}

TEST(SdkLocatorTests, RejectsPlatformSdkMissingAConfiguration) {
    TemporarySdkDirectory directory;
    const auto sdkPath = directory.Path() / "Windows-x64" / "1.4.0";
    const auto platformPath = directory.Path() / "Windows-x64";
    std::filesystem::create_directories(sdkPath / "Debug");
    std::filesystem::create_directories(sdkPath / "include" / "NeoWPF");
    std::filesystem::create_directories(sdkPath / "third_party" / "include" / "NeoNaml");
    std::ofstream(sdkPath / ".include_paths") << "include\nthird_party/include\n";
    std::ofstream(platformPath / ".version") << "1.4.0";
    std::ofstream(sdkPath / "Debug" / "NeoWPF.Windows.Debug.1.0.0.lib");

    NeoPM::Core::SdkLocator locator(directory.Path(), "Windows");

    EXPECT_FALSE(locator.FindLibrary("release", "x64"));
    EXPECT_FALSE(locator.HasAnySdk());
}

TEST(SdkLocatorTests, DoesNotRecognizeThePreviousSdkLayout) {
    TemporarySdkDirectory directory;
    const auto sdkPath = directory.Path() / "debug-x64";
    std::filesystem::create_directories(sdkPath / "include" / "NeoWPF");
    std::filesystem::create_directories(sdkPath / "lib");
    std::ofstream(sdkPath / "lib" / "NeoWPF.lib");

    NeoPM::Core::SdkLocator locator(directory.Path(), "Windows");

    EXPECT_FALSE(locator.FindSdk("x64"));
    EXPECT_FALSE(locator.HasAnySdk());
}
