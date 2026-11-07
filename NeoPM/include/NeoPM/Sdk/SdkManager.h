/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>
#include <string>
#include <optional>

namespace NeoPM::Core {

class SdkManager {
public:
    explicit SdkManager();

    bool RefreshSdks();

    std::optional<std::filesystem::path> GetSdkPath() const;

    std::optional<std::filesystem::path> GetSdkLibraryPath(const std::string& configuration) const;

    std::optional<std::filesystem::path> GetSdkIncludePath() const;

    std::filesystem::path GetSdkRoot() const { return m_sdkRoot; }

    bool HasLocalSdks() const;

private:
    struct Release {
        std::string version;
        std::string platform;
        std::string architecture;
        std::string url;
        std::string debugLibraryPath;
        std::string releaseLibraryPath;
        std::string licensePath;
        std::string includePath;
    };

    bool DownloadAndExtractRelease(const Release& release);
    bool ExtractZip(const std::filesystem::path& zipPath, const std::filesystem::path& extractPath);

    std::filesystem::path m_sdkRoot;
    std::string m_sdkPlatform;
    std::string m_sdkArchitecture;
};

}
