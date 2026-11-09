/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>
#include <string>

namespace NeoPM::Core {

class SdkVersionManager {
public:
    explicit SdkVersionManager(const std::filesystem::path& sdkRoot);

    // Get the stored version of an installed SDK
    std::string GetInstalledVersion(const std::string& platform, const std::string& architecture) const;

    // Save the version of an installed SDK
    void SaveInstalledVersion(const std::string& platform, const std::string& architecture, const std::string& version);

    std::filesystem::path GetVersionDirectory(
        const std::string& platform,
        const std::string& architecture,
        const std::string& version) const;

private:
    std::filesystem::path m_sdkRoot;
    std::filesystem::path GetVersionFilePath(const std::string& platform, const std::string& architecture) const;
};

}
