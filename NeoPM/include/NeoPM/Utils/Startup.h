#pragma once

#include <filesystem>

namespace NeoPM::Utils {
    class Startup {
    public:
        static bool InstallUserEntry(const std::filesystem::path& executablePath,
                                     const std::filesystem::path& configPath);
    };
}