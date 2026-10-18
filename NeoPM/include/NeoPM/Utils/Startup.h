/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>

namespace NeoPM::Utils {
    class Startup {
    public:
        static bool InstallUserEntry(const std::filesystem::path& executablePath,
                                     const std::filesystem::path& configPath);
    };
}