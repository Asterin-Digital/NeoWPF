/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Sdk/SdkManifest.h>

#include <filesystem>

namespace NeoPM::Core {

class SdkArchiveInstaller final {
public:
    static bool Install(
        const SdkRelease& release,
        const std::filesystem::path& archiveRoot,
        const std::filesystem::path& sdkRoot);
};

}