#include <NeoPM/Utils/FileSystem.h>

#include <Winux/Winux.h>

#include <boost/dll/runtime_symbol_info.hpp>

namespace NeoPM::Utils {
    namespace {
        std::unique_ptr<Winux::Contracts::IPlatform> CreatePlatform() {
            return Winux::Platform::create();
        }

        std::optional<std::filesystem::path> ReadEnvironmentPath(
            Winux::Contracts::IEnvironment& environment,
            const wchar_t* name) {
            const auto value = environment.get_env(name);
            if (value.failed() || value.value().empty()) {
                return std::nullopt;
            }
            return std::filesystem::path(value.value());
        }
    }

    bool FileSystem::DirectoryExists(const std::filesystem::path& path) {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    }

    bool FileSystem::FileExists(const std::filesystem::path& path) {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    }

    bool FileSystem::IsEmpty(const std::filesystem::path& path) {
        if (!DirectoryExists(path)) {
            return false;
        }
        return std::filesystem::is_empty(path);
    }

    std::optional<std::filesystem::path> FileSystem::GetUserDocumentsPath() {
        const auto platform = CreatePlatform();
        if (const auto documents = ReadEnvironmentPath(platform->environment(), L"XDG_DOCUMENTS_DIR")) {
            if (DirectoryExists(*documents)) return documents;
        }
        if (const auto home = GetUserHomePath()) {
            const auto path = *home / "Documents";
            if (DirectoryExists(path)) return path;
        }
        return std::nullopt;
    }

    std::optional<std::filesystem::path> FileSystem::GetUserHomePath() {
        const auto platform = CreatePlatform();
        const auto home = platform->file_system().home();
        if (home.succeeded() && DirectoryExists(home.value())) {
            return home.value();
        }
        return std::nullopt;
    }

    std::optional<std::filesystem::path> FileSystem::GetUserDataPath() {
        const auto platform = CreatePlatform();
        const auto data = platform->file_system().app_data().local().start();
        if (data.succeeded()) {
            return data.value();
        }
        return std::nullopt;
    }

    std::optional<std::filesystem::path> FileSystem::GetUserConfigPath() {
        const auto platform = CreatePlatform();
        if (const auto config = ReadEnvironmentPath(platform->environment(), L"APPDATA")) {
            return config;
        }
        if (const auto config = ReadEnvironmentPath(platform->environment(), L"XDG_CONFIG_HOME")) {
            return config;
        }
        if (const auto home = GetUserHomePath()) {
            return *home / ".config";
        }
        return std::nullopt;
    }

    bool FileSystem::CreateDirectory(const std::filesystem::path& path) {
        try {
            return std::filesystem::create_directory(path);
        }
        catch (const std::filesystem::filesystem_error&) {
            return false;
        }
    }

    bool FileSystem::CreateDirectories(const std::filesystem::path& path) {
        try {
            return std::filesystem::create_directories(path);
        }
        catch (const std::filesystem::filesystem_error&) {
            return false;
        }
    }

    bool FileSystem::DeleteDirectory(const std::filesystem::path& path, bool recursive) {
        try {
            if (recursive) {
                std::filesystem::remove_all(path);
            }
            else {
                std::filesystem::remove(path);
            }
            return true;
        }
        catch (const std::filesystem::filesystem_error&) {
            return false;
        }
    }

    bool FileSystem::DeleteFile(const std::filesystem::path& path) {
        try {
            return std::filesystem::remove(path) != 0;
        }
        catch (const std::filesystem::filesystem_error&) {
            return false;
        }
    }

    std::optional<std::filesystem::path> FileSystem::GetExecutablePath() {
        try {
            const auto executable = boost::dll::program_location();
            if (executable.empty()) return std::nullopt;
            return std::filesystem::path(executable.string());
        }
        catch (...) {
            return std::nullopt;
        }
    }

    std::optional<std::filesystem::path> FileSystem::GetExecutableDirectory() {
        auto execPath = GetExecutablePath();
        if (execPath) {
            return execPath->parent_path();
        }
        return std::nullopt;
    }
}
