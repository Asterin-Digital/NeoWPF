/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Sdk/SdkManifest.h>

#include <filesystem>
#include <string>
#include <optional>
#include <vector>

namespace NeoPM::Core {

struct SdkInstallation {
    std::string version;
    std::vector<std::filesystem::path> includeDirectories;
    std::filesystem::path debugLibraryPath;
    std::filesystem::path releaseLibraryPath;
};

class SdkManager {
public:
    explicit SdkManager();

    bool RefreshSdks();

    std::optional<std::filesystem::path> GetSdkPath() const;

    std::optional<std::filesystem::path> GetSdkLibraryPath(const std::string& configuration) const;

    std::vector<std::filesystem::path> GetSdkIncludeDirectories() const;

    std::optional<SdkInstallation> GetSdkInstallation(
        const std::string& platform,
        const std::string& architecture) const;

    std::filesystem::path GetSdkRoot() const { return m_sdkRoot; }

    bool HasLocalSdks() const;

private:
    bool DownloadAndExtractRelease(const SdkRelease& release);
    bool ExtractZip(const std::filesystem::path& zipPath, const std::filesystem::path& extractPath);

    std::filesystem::path m_sdkRoot;
    std::string m_sdkPlatform;
    std::string m_sdkArchitecture;
};

}
