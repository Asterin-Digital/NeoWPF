/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>
#include <string>
#include <optional>
#include <vector>

namespace NeoPM::Core {

class SdkLocator {
public:
    SdkLocator(const std::filesystem::path& sdkRoot, std::string platform);
    std::optional<std::filesystem::path> FindSdk(const std::string& architecture) const;
    std::optional<std::filesystem::path> FindSdk(
        const std::string& architecture,
        const std::string& version) const;
    std::optional<std::filesystem::path> FindLibrary(const std::string& configuration, const std::string& architecture) const;
    std::optional<std::filesystem::path> FindLibrary(
        const std::string& configuration,
        const std::string& architecture,
        const std::string& version) const;
    std::vector<std::filesystem::path> FindIncludeDirectories(const std::string& architecture) const;
    std::vector<std::filesystem::path> FindIncludeDirectories(
        const std::string& architecture,
        const std::string& version) const;
    bool HasAnySdk() const;

private:
    std::filesystem::path m_sdkRoot;
    std::string m_platform;
};

}
