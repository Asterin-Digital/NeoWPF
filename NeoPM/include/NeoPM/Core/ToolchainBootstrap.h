/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>
#include <string>

namespace NeoPM::Core {
    class ToolchainBootstrap {
    public:
        static bool EnsureInstalled();
        static std::filesystem::path GetLocalAppDataDirectory();
        static std::filesystem::path GetToolDirectory();
        static std::filesystem::path GetTargetsFilePath();
        static std::filesystem::path GetCMakeModulePath();
        static std::filesystem::path GetToolExecutablePath();
        static std::string BuildTargetsXml(const std::filesystem::path& toolExecutablePath);
        static std::string BuildResourceSyncCMakeModule(const std::filesystem::path& toolExecutablePath);
    };
}
