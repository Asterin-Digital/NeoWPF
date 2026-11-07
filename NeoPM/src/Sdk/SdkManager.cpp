#include <NeoPM/Sdk/SdkManager.h>
#include <NeoPM/Sdk/SdkVersionManager.h>
#include <NeoPM/Sdk/SdkLocator.h>
#include <NeoPM/Utils/Http.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>
#include <NeoPM/Utils/TextEncoding.h>
#include <NeoPM/Utils/Archive.h>

#include <nlohmann/json.hpp>

#include <sstream>
#include <algorithm>
#include <cstring>
#include <fstream>

using json = nlohmann::json;

namespace NeoPM::Core {

SdkManager::SdkManager()
        : m_sdkPlatform(NEOPM_SDK_PLATFORM),
            m_sdkArchitecture(NEOPM_SDK_ARCHITECTURE)
{
    m_sdkRoot = Utils::FileSystem::GetUserDataPath().value_or(
        std::filesystem::current_path()) / "NeoWPF" / "SDK";
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
        auto manifest = json::parse(manifestJson);

        if (!manifest.contains("releases") || !manifest["releases"].is_array()) {
            logger.Error("SdkManager", "SDK manifest must contain a releases array");
            return false;
        }

        for (const auto& rel : manifest["releases"]) {
            if (!rel.is_object() || !rel.contains("architecture") ||
                !rel.contains("platform") || !rel.contains("url") ||
                !rel.contains("version") || !rel.contains("files") ||
                !rel["files"].is_object()) {
                continue;
            }

            const auto& files = rel["files"];
            if (!files.contains("debug") || !files["debug"].is_string() ||
                files["debug"].get<std::string>().empty() ||
                !files.contains("release") || !files["release"].is_string() ||
                files["release"].get<std::string>().empty() ||
                !files.contains("license") || !files["license"].is_string() ||
                !files.contains("include") || !files["include"].is_string()) {
                logger.Warning("SdkManager", "Rejecting platform SDK without Debug and Release files");
                continue;
            }

            Release candidate;
            candidate.version = rel["version"];
            candidate.platform = rel["platform"];
            candidate.architecture = rel["architecture"];
            candidate.url = rel["url"];
            candidate.debugLibraryPath = files["debug"];
            candidate.releaseLibraryPath = files["release"];
            candidate.licensePath = files["license"];
            candidate.includePath = files["include"];

            const auto debugPath = std::filesystem::path(candidate.debugLibraryPath);
            const auto releasePath = std::filesystem::path(candidate.releaseLibraryPath);
            const auto libraryExtension = candidate.platform == "Windows" ? ".lib" : ".a";
            const auto debugFileName = candidate.platform == "Android"
                ? "libNeoWPF.a"
                : "NeoWPF." + candidate.platform + ".Debug." + candidate.version + libraryExtension;
            const auto releaseFileName = candidate.platform == "Android"
                ? "libNeoWPF.a"
                : "NeoWPF." + candidate.platform + ".Release." + candidate.version + libraryExtension;
            if ((candidate.platform != "Windows" && candidate.platform != "Linux" &&
                 candidate.platform != "Android") ||
                (candidate.platform == "Android" && candidate.architecture != "arm64") ||
                (candidate.platform != "Android" && candidate.architecture != "x64") ||
                debugPath.parent_path() != "Debug" || releasePath.parent_path() != "Release" ||
                debugPath.filename() != debugFileName || releasePath.filename() != releaseFileName ||
                candidate.licensePath != "LICENSE" || candidate.includePath != "include") {
                logger.Warning("SdkManager", "Rejecting platform SDK with an invalid package layout");
                continue;
            }

            SdkVersionManager versionMgr(m_sdkRoot);
            auto localVersion = versionMgr.GetInstalledVersion(candidate.platform, candidate.architecture);
            SdkLocator localLocator(m_sdkRoot, candidate.platform);
            const bool localSdkComplete =
                localLocator.FindIncludeDirectory(candidate.architecture) &&
                localLocator.FindLibrary("debug", candidate.architecture) &&
                localLocator.FindLibrary("release", candidate.architecture);
            if (localVersion == candidate.version && localSdkComplete) {
                logger.Info("SdkManager", std::string("SDK up-to-date: ") + candidate.platform + "-" + candidate.architecture + " v" + candidate.version);
                continue;
            }

            if (!DownloadAndExtractRelease(candidate)) {
                logger.Warning("SdkManager", std::string("Rejected incomplete platform SDK: ") + candidate.platform + "-" + candidate.architecture);
                continue;
            }

            logger.Info("SdkManager", std::string("SDK installed: ") + candidate.platform + "-" + candidate.architecture + " v" + candidate.version);
        }

        return true;
    }
    catch (const std::exception& e) {
        logger.Error("SdkManager", std::string("Failed to parse SDK manifest: ") + e.what());
        return false;
    }
}

bool SdkManager::DownloadAndExtractRelease(const Release& release) {
    auto& logger = Utils::Logger::Instance();

    auto sdkFolder = m_sdkRoot / (release.platform + "-" + release.architecture);
    std::error_code ec;
    std::filesystem::create_directories(sdkFolder, ec);

    auto zipPath = sdkFolder / ("NeoWPF-" + release.version + ".zip");

    logger.Info("SdkManager", "Downloading " + release.platform + " SDK from " + release.url);

    std::wstring wideUrl(release.url.begin(), release.url.end());
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


    if (!ExtractZip(zipPath, sdkFolder)) {
        logger.Error("SdkManager", "Failed to extract platform SDK archive");
        return false;
    }

    const auto debugLibrary = sdkFolder / release.debugLibraryPath;
    const auto releaseLibrary = sdkFolder / release.releaseLibraryPath;
    const auto license = sdkFolder / release.licensePath;
    const auto includeDirectory = sdkFolder / release.includePath;
    if (!std::filesystem::is_regular_file(debugLibrary) ||
        !std::filesystem::is_regular_file(releaseLibrary) ||
        !std::filesystem::is_regular_file(license) ||
        !std::filesystem::is_directory(includeDirectory) ||
        !std::filesystem::is_directory(includeDirectory / "NeoWPF") ||
        !std::filesystem::is_directory(includeDirectory / "NeoNaml")) {
        logger.Error("SdkManager", "Platform SDK must contain Debug and Release libraries, include/NeoWPF, include/NeoNaml, and LICENSE");
        std::filesystem::remove_all(sdkFolder, ec);
        return false;
    }

    logger.Info("SdkManager", "Platform SDK extracted to " + sdkFolder.string());

    std::filesystem::remove(zipPath, ec);

    // Save version to mark this SDK as installed
    SdkVersionManager versionMgr(m_sdkRoot);
    versionMgr.SaveInstalledVersion(release.platform, release.architecture, release.version);

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

std::optional<std::filesystem::path> SdkManager::GetSdkIncludePath() const {
    SdkLocator locator(m_sdkRoot, m_sdkPlatform);
    return locator.FindIncludeDirectory(m_sdkArchitecture);
}

bool SdkManager::HasLocalSdks() const {
    SdkLocator locator(m_sdkRoot, m_sdkPlatform);
    return locator.HasAnySdk();
}

}
