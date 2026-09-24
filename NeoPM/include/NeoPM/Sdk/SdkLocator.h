/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>
#include <string>
#include <optional>

namespace NeoPM::Core {

class SdkLocator {
public:
    explicit SdkLocator(const std::filesystem::path& sdkRoot);
    std::optional<std::filesystem::path> FindSdk(const std::string& channel, const std::string& architecture) const;
    std::optional<std::filesystem::path> FindLibrary(const std::string& channel, const std::string& architecture) const;
    std::optional<std::filesystem::path> FindIncludeDirectory(const std::string& channel, const std::string& architecture) const;
    bool HasAnySdk() const;

private:
    std::filesystem::path m_sdkRoot;
};

}
