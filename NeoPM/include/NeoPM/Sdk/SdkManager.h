/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>
#include <string>
#include <optional>

namespace NeoPM::Core {

struct SdkInfo {
    std::string version;
    std::string channel;
    std::string platform;
    std::string architecture;
    std::string downloadUrl;
    std::filesystem::path localPath;

    std::string GetFolderName() const {
        return channel + "-" + architecture;
    }
};

class SdkManager {
public:
    explicit SdkManager();

    bool RefreshSdks();

    std::optional<std::filesystem::path> GetSdkPath(const std::string& channel, const std::string& architecture = "x64") const;

    std::optional<std::filesystem::path> GetSdkLibraryPath(const std::string& channel, const std::string& architecture = "x64") const;

    std::optional<std::filesystem::path> GetSdkIncludePath(const std::string& channel, const std::string& architecture = "x64") const;

    std::filesystem::path GetSdkRoot() const { return m_sdkRoot; }

    bool HasLocalSdks() const;

private:
    struct Release {
        std::string version;
        std::string channel;
        std::string platform;
        std::string architecture;
        std::string url;
        std::string libraryPath;
        std::string licensePath;
        std::string includePath;
    };

    bool DownloadAndExtractRelease(const Release& release);
    bool ExtractZip(const std::filesystem::path& zipPath, const std::filesystem::path& extractPath);

    std::filesystem::path m_sdkRoot;
};

}
