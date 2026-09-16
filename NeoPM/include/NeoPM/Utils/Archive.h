#pragma once

#include <filesystem>

namespace NeoPM::Utils {
    class Archive {
    public:
        static bool ExtractZip(const std::filesystem::path& archivePath,
                               const std::filesystem::path& destinationPath);
    };
}