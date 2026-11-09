#include <NeoPM/Sdk/SdkManager.h>
#include <NeoPM/Sdk/SdkArchiveInstaller.h>
#include <NeoPM/Sdk/SdkVersionManager.h>
#include <NeoPM/Sdk/SdkLocator.h>
#include <NeoPM/Core/ToolchainBootstrap.h>
#include <NeoPM/Utils/Http.h>
#include <NeoPM/Utils/Logger.h>
#include <NeoPM/Utils/TextEncoding.h>
#include <NeoPM/Utils/Archive.h>

#include <array>
#include <fstream>

namespace NeoPM::Core {

SdkManager::SdkManager()
        : m_sdkPlatform(NEOPM_SDK_PLATFORM),
            m_sdkArchitecture(NEOPM_SDK_ARCHITECTURE)
{
    m_sdkRoot = ToolchainBootstrap::GetToolDirectory() / "SDK";
}

bool SdkManager::RefreshSdks() {
    auto& logger = Utils::Logger::Instance();

    std::error_code ec;
    std::filesystem::create_directories(m_sdkRoot, ec);

    logger.Info("SdkManager", "Refreshing NeoWPF SDKs...");

    Net::CurlClient client;
    std::string manifestJson;
    const std::string manifestUrl = Net::NEOWPF_SDK_RELEASES_URL;
    const std::wstring wideManifestUrl(manifestUrl.begin(), manifestUrl.end());

    auto outcome = client.DownloadToBuffer(
        wideManifestUrl,
        manifestJson,
        [&](uint64_t got, uint64_t total) {
            logger.Debug("SdkManager", "Downloading manifest: " + std::to_string(got) + "/" + std::to_string(total) + " bytes");
            return true;
        });

    if (!outcome.Ok()) {
        logger.Warning("SdkManager", "Failed to download SDK manifest, using local SDKs if available");
        if (HasLocalSdks()) {
            logger.Info("SdkManager", "Local SDKs found, proceeding");
            return true;
        }
        logger.Error("SdkManager", "No local SDKs available");
        return false;
    }

    logger.Debug("SdkManager", "SDK manifest downloaded: " + std::to_string(manifestJson.size()) + " bytes");

    try {
        const std::array<std::pair<std::string, std::string>, 3> targets{{
            {"Windows", "x64"},
            {"Linux", "x64"},
            {"Android", "arm64-v8a"}}};
        SdkVersionManager versionManager(m_sdkRoot);

        for (const auto& [platform, architecture] : targets) {
            const auto candidate = SdkManifestParser::Parse(manifestJson, platform, architecture);
            if (!candidate) continue;

            SdkLocator locator(m_sdkRoot, platform);
            const bool versionAlreadyInstalled =
                !locator.FindIncludeDirectories(architecture, candidate->version).empty() &&
                locator.FindLibrary("debug", architecture, candidate->version) &&
                locator.FindLibrary("release", architecture, candidate->version);
            if (versionAlreadyInstalled) {
                versionManager.SaveInstalledVersion(platform, architecture, candidate->version);
                continue;
            }

            if (!DownloadAndExtractRelease(*candidate)) {
                logger.Warning("SdkManager", "Rejected incomplete platform SDK: " + platform + "-" + architecture);
                continue;
            }

            logger.Info("SdkManager", "SDK installed: " + platform + "-" + architecture + " v" + candidate->version);
        }

        return HasLocalSdks();
    }
    catch (const std::exception& e) {
        logger.Error("SdkManager", std::string("Failed to parse SDK manifest: ") + e.what());
        return false;
    }
}

bool SdkManager::DownloadAndExtractRelease(const SdkRelease& release) {
    auto& logger = Utils::Logger::Instance();

    SdkVersionManager versionManager(m_sdkRoot);
    const auto sdkPlatformRoot = m_sdkRoot / (release.platform + "-" + release.architecture);
    const auto sdkFolder = versionManager.GetVersionDirectory(
        release.platform, release.architecture, release.version);
    std::error_code ec;
    std::filesystem::create_directories(sdkPlatformRoot, ec);
    if (ec) return false;

    const auto zipPath = sdkPlatformRoot / ("NeoWPF-" + release.version + ".zip");

    logger.Info("SdkManager", "Downloading " + release.platform + " SDK from " + release.downloadUrl);

    std::wstring wideUrl(release.downloadUrl.begin(), release.downloadUrl.end());
    std::wstring wideZipPath = zipPath.wstring();

    Net::CurlClient client;
    auto outcome = client.DownloadFile(
        wideUrl,
        wideZipPath,
        [&](uint64_t got, uint64_t total) {
            if (total > 0) {
                int percent = static_cast<int>((got * 100) / total);
                logger.Debug("SdkManager", "Download: " + std::to_string(percent) + "%");
            }
            return true;
        });

    if (!outcome.Ok()) {
        logger.Error("SdkManager", "Failed to download SDK: " + Utils::WideToUtf8(outcome.message));
        return false;
    }

    logger.Info("SdkManager", "SDK downloaded to " + zipPath.string());

    std::filesystem::remove_all(sdkFolder, ec);
    ec.clear();
    std::filesystem::create_directories(sdkFolder, ec);
    if (ec) return false;

    const auto extractPath = sdkFolder / ".combined";
    std::filesystem::remove_all(extractPath, ec);
    ec.clear();
    if (!ExtractZip(zipPath, extractPath)) {
        logger.Error("SdkManager", "Failed to extract combined SDK archive");
        return false;
    }

    const bool copied = SdkArchiveInstaller::Install(release, extractPath, sdkFolder);
    std::filesystem::remove_all(extractPath, ec);
    if (!copied) {
        logger.Error("SdkManager", "Combined SDK archive is missing a requested library or include directory");
        std::filesystem::remove(zipPath, ec);
        return false;
    }

    logger.Info("SdkManager", "SDK platform archive extracted to " + sdkFolder.string());
    std::filesystem::remove(zipPath, ec);

    // Save version to mark this SDK as installed
    versionManager.SaveInstalledVersion(release.platform, release.architecture, release.version);

    return true;
}

bool SdkManager::ExtractZip(
    const std::filesystem::path& zipPath,
    const std::filesystem::path& extractPath)
{
    if (!std::filesystem::exists(zipPath)) {
        return false;
    }
    return Utils::Archive::ExtractZip(zipPath, extractPath);
}

std::optional<std::filesystem::path> SdkManager::GetSdkPath() const {
    SdkLocator locator(m_sdkRoot, m_sdkPlatform);
    return locator.FindSdk(m_sdkArchitecture);
}

std::optional<std::filesystem::path> SdkManager::GetSdkLibraryPath(const std::string& configuration) const {
    SdkLocator locator(m_sdkRoot, m_sdkPlatform);
    return locator.FindLibrary(configuration, m_sdkArchitecture);
}

std::vector<std::filesystem::path> SdkManager::GetSdkIncludeDirectories() const {
    SdkLocator locator(m_sdkRoot, m_sdkPlatform);
    return locator.FindIncludeDirectories(m_sdkArchitecture);
}

std::optional<SdkInstallation> SdkManager::GetSdkInstallation(
    const std::string& platform,
    const std::string& architecture) const
{
    SdkVersionManager versionManager(m_sdkRoot);
    const auto version = versionManager.GetInstalledVersion(platform, architecture);
    if (version.empty()) return std::nullopt;

    SdkLocator locator(m_sdkRoot, platform);
    auto includeDirectories = locator.FindIncludeDirectories(architecture, version);
    auto debugLibraryPath = locator.FindLibrary("debug", architecture, version);
    auto releaseLibraryPath = locator.FindLibrary("release", architecture, version);
    if (includeDirectories.empty() || !debugLibraryPath || !releaseLibraryPath) {
        return std::nullopt;
    }

    return SdkInstallation{
        version,
        std::move(includeDirectories),
        std::move(*debugLibraryPath),
        std::move(*releaseLibraryPath)};
}

bool SdkManager::HasLocalSdks() const {
    SdkLocator locator(m_sdkRoot, m_sdkPlatform);
    return locator.HasAnySdk();
}

}
