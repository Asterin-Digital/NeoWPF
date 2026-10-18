/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>
#include <optional>

namespace NeoPM::Utils {
    class FileSystem {
    public:
        static bool DirectoryExists(const std::filesystem::path& path);
        static bool FileExists(const std::filesystem::path& path);
        static bool IsEmpty(const std::filesystem::path& path);

        static std::optional<std::filesystem::path> GetUserDocumentsPath();
        static std::optional<std::filesystem::path> GetUserHomePath();
        static std::optional<std::filesystem::path> GetUserDataPath();
        static std::optional<std::filesystem::path> GetUserConfigPath();

        static bool CreateDirectory(const std::filesystem::path& path);
        static bool CreateDirectories(const std::filesystem::path& path);

        static bool DeleteDirectory(const std::filesystem::path& path, bool recursive = false);
        static bool DeleteFile(const std::filesystem::path& path);

        static std::optional<std::filesystem::path> GetExecutablePath();
        static std::optional<std::filesystem::path> GetExecutableDirectory();
    };
}
