#include <NeoPM/Sdk/SdkLocator.h>
#include <NeoPM/Sdk/SdkManifest.h>
#include <NeoPM/Sdk/SdkVersionManager.h>
#include <NeoPM/Utils/Logger.h>

#include <algorithm>
#include <fstream>
#include <utility>

namespace NeoPM::Core {

SdkLocator::SdkLocator(const std::filesystem::path& sdkRoot, std::string platform)
    : m_sdkRoot(sdkRoot), m_platform(std::move(platform))
{
}

std::optional<std::filesystem::path> SdkLocator::FindSdk(
    const std::string& architecture) const
{
    const auto version = SdkVersionManager(m_sdkRoot).GetInstalledVersion(m_platform, architecture);
    return version.empty() ? std::nullopt : FindSdk(architecture, version);
}

std::optional<std::filesystem::path> SdkLocator::FindSdk(
    const std::string& architecture,
    const std::string& version) const
{
    const std::filesystem::path versionPath(version);
    if (!SdkManifestParser::IsSafeRelativePath(version) ||
        versionPath.has_parent_path() || versionPath.filename().string() != version) {
        return std::nullopt;
    }

    const auto sdkFolder = SdkVersionManager(m_sdkRoot).GetVersionDirectory(
        m_platform, architecture, version);
    return std::filesystem::is_directory(sdkFolder)
        ? std::optional<std::filesystem::path>(sdkFolder)
        : std::nullopt;
}

std::optional<std::filesystem::path> SdkLocator::FindLibrary(
    const std::string& configuration,
    const std::string& architecture) const
{
    auto sdkPath = FindSdk(architecture);
    if (!sdkPath) return std::nullopt;
    const auto version = SdkVersionManager(m_sdkRoot).GetInstalledVersion(m_platform, architecture);
    return FindLibrary(configuration, architecture, version);
}

std::optional<std::filesystem::path> SdkLocator::FindLibrary(
    const std::string& configuration,
    const std::string& architecture,
    const std::string& version) const
{
    auto sdkPath = FindSdk(architecture, version);
    if (!sdkPath) {
        return std::nullopt;
    }

    try {
        const std::string configurationName = configuration == "debug" ? "Debug" :
            configuration == "release" ? "Release" : "";
        if (configurationName.empty()) {
            return std::nullopt;
        }

        const auto configurationPath = sdkPath.value() / configurationName;
        for (const auto& entry : std::filesystem::directory_iterator(configurationPath)) {
            if (entry.is_regular_file() &&
                (entry.path().extension() == ".lib" || entry.path().extension() == ".a")) {
                return entry.path();
            }
        }
    }
    catch (const std::exception& e) {
        auto& logger = Utils::Logger::Instance();
        logger.Debug("SdkLocator", "Error searching platform configuration library: " + std::string(e.what()));
    }

    return std::nullopt;
}

std::vector<std::filesystem::path> SdkLocator::FindIncludeDirectories(
    const std::string& architecture) const
{
    auto sdkPath = FindSdk(architecture);
    if (!sdkPath) return {};
    const auto version = SdkVersionManager(m_sdkRoot).GetInstalledVersion(m_platform, architecture);
    return FindIncludeDirectories(architecture, version);
}

std::vector<std::filesystem::path> SdkLocator::FindIncludeDirectories(
    const std::string& architecture,
    const std::string& version) const
{
    auto sdkPath = FindSdk(architecture, version);
    if (!sdkPath) {
        return {};
    }

    try {
        std::ifstream includePathsFile(sdkPath.value() / ".include_paths");
        if (!includePathsFile) {
            return {};
        }

        std::vector<std::filesystem::path> includeDirectories;
        std::string relativePath;
        while (std::getline(includePathsFile, relativePath)) {
            if (!SdkManifestParser::IsSafeRelativePath(relativePath)) return {};
            const auto includePath = sdkPath.value() / relativePath;
            if (!std::filesystem::is_directory(includePath)) return {};
            includeDirectories.push_back(includePath);
        }

        const auto hasDirectory = [&includeDirectories](const char* name) {
            return std::any_of(includeDirectories.begin(), includeDirectories.end(), [name](const auto& path) {
                return std::filesystem::is_directory(path / name);
            });
        };
        if (!includeDirectories.empty() && hasDirectory("NeoWPF") && hasDirectory("NeoNaml")) {
            return includeDirectories;
        }
    }
    catch (const std::exception& e) {
        auto& logger = Utils::Logger::Instance();
        logger.Debug("SdkLocator", "Error searching for include directory: " + std::string(e.what()));
    }

    return {};
}

bool SdkLocator::HasAnySdk() const {
    if (!std::filesystem::exists(m_sdkRoot)) {
        return false;
    }

    try {
        for (const auto& entry : std::filesystem::directory_iterator(m_sdkRoot)) {
            if (!entry.is_directory() ||
                entry.path().filename().string().rfind(m_platform + "-", 0) != 0) {
                continue;
            }

            auto architecture = entry.path().filename().string().substr(m_platform.size() + 1);
            if (!FindIncludeDirectories(architecture).empty() &&
                FindLibrary("debug", architecture) &&
                FindLibrary("release", architecture)) {
                return true;
            }
        }
    }
    catch (const std::exception& e) {
        auto& logger = Utils::Logger::Instance();
        logger.Debug("SdkLocator", "Error checking for local SDKs: " + std::string(e.what()));
    }

    return false;
}

}
