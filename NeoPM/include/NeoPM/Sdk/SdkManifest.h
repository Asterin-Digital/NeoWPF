/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace NeoPM::Core {

struct SdkRelease {
    std::string version;
    std::string platform;
    std::string architecture;
    std::string downloadUrl;
    std::string debugLibraryPath;
    std::string releaseLibraryPath;
    std::string licensePath;
    std::vector<std::string> includePaths;
};

class SdkManifestParser {
public:
    static std::optional<SdkRelease> Parse(
        const std::string& manifestJson,
        const std::string& platform,
        const std::string& architecture);

    static bool IsSafeRelativePath(const std::string& value);
};

}