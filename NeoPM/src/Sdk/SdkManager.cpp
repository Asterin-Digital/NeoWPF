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

    auto outcome = client.DownloadToBuffer(
        L"https://raw.githubusercontent.com/NeoWPF/NeoWPF/main/sdk-releases.json",
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
            logger.Warning("SdkManager", "No releases found in manifest");
            return true;
        }

        for (const auto& rel : manifest["releases"]) {
            if (!rel.contains("channel") || !rel.contains("architecture") ||
                !rel.contains("platform") || !rel.contains("url") ||
                !rel.contains("version")) {
                continue;
            }

            Release candidate;
            candidate.version = rel["version"];
            candidate.channel = rel["channel"];
            candidate.platform = rel["platform"];
            candidate.architecture = rel["architecture"];
            candidate.url = rel["url"];

            if (rel.contains("files")) {
                if (rel["files"].contains("library")) {
                    candidate.libraryPath = rel["files"]["library"];
                }
                if (rel["files"].contains("license")) {
                    candidate.licensePath = rel["files"]["license"];
                }
                if (rel["files"].contains("include")) {
                    candidate.includePath = rel["files"]["include"];
                }
            }

            // Check if local version matches remote version
            SdkVersionManager versionMgr(m_sdkRoot);
            auto localVersion = versionMgr.GetInstalledVersion(candidate.channel, candidate.architecture);
            if (localVersion == candidate.version) {
                logger.Info("SdkManager", std::string("SDK up-to-date: ") + candidate.channel + "-" + candidate.architecture + " v" + candidate.version);
                continue;
            }

            if (!DownloadAndExtractRelease(candidate)) {
                logger.Warning("SdkManager", std::string("Failed to download/extract ") + candidate.channel + "-" + candidate.architecture + " release");
                continue;
            }

            logger.Info("SdkManager", std::string("SDK installed: ") + candidate.channel + "-" + candidate.architecture + " v" + candidate.version);
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

    auto sdkFolder = m_sdkRoot / (release.channel + "-" + release.architecture);
    std::error_code ec;
    std::filesystem::create_directories(sdkFolder, ec);

    auto zipPath = sdkFolder / ("NeoWPF-" + release.version + ".zip");

    logger.Info("SdkManager", "Downloading " + release.channel + " SDK from " + release.url);

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
        logger.Error("SdkManager", "Failed to extract SDK zip");
        return false;
    }

    logger.Info("SdkManager", "SDK extracted to " + sdkFolder.string());

    std::filesystem::remove(zipPath, ec);

    // Save version to mark this SDK as installed
    SdkVersionManager versionMgr(m_sdkRoot);
    versionMgr.SaveInstalledVersion(release.channel, release.architecture, release.version);

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

std::optional<std::filesystem::path> SdkManager::GetSdkPath(const std::string& channel, const std::string& architecture) const {
    SdkLocator locator(m_sdkRoot);
    return locator.FindSdk(channel, architecture);
}

std::optional<std::filesystem::path> SdkManager::GetSdkLibraryPath(const std::string& channel, const std::string& architecture) const {
    SdkLocator locator(m_sdkRoot);
    return locator.FindLibrary(channel, architecture);
}

std::optional<std::filesystem::path> SdkManager::GetSdkIncludePath(const std::string& channel, const std::string& architecture) const {
    SdkLocator locator(m_sdkRoot);
    return locator.FindIncludeDirectory(channel, architecture);
}

bool SdkManager::HasLocalSdks() const {
    SdkLocator locator(m_sdkRoot);
    return locator.HasAnySdk();
}

}
