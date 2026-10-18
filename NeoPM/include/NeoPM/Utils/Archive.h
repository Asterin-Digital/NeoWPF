/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>

namespace NeoPM::Utils {
    class Archive {
    public:
        static bool ExtractZip(const std::filesystem::path& archivePath,
                               const std::filesystem::path& destinationPath);
    };
}