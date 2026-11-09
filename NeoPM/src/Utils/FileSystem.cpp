#include <NeoPM/Utils/FileSystem.h>

#include <Winux/Winux.h>

#include <boost/dll/runtime_symbol_info.hpp>

#ifdef CreateDirectory
#undef CreateDirectory
#endif
#ifdef DeleteFile
#undef DeleteFile
#endif

namespace NeoPM::Utils {
    namespace {
        std::optional<std::filesystem::path> ReadEnvironmentPath(
            Winux::Contracts::IEnvironment& environment,
            const wchar_t* name) {
            const auto value = environment.GetEnv(name);
            if (value.Failed() || value.Value().empty()) {
                return std::nullopt;
            }
            return std::filesystem::path(value.Value());
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
        if (const auto documents = ReadEnvironmentPath(Winux::Get<Winux::Environment>(), L"XDG_DOCUMENTS_DIR")) {
            if (DirectoryExists(*documents)) return documents;
        }
        if (const auto home = GetUserHomePath()) {
            const auto path = *home / "Documents";
            if (DirectoryExists(path)) return path;
        }
        return std::nullopt;
    }

    std::optional<std::filesystem::path> FileSystem::GetUserHomePath() {
        const auto home = Winux::Get<Winux::FileSystem>().Home();
        if (home.Succeeded() && DirectoryExists(home.Value())) {
            return home.Value();
        }
        return std::nullopt;
    }

    std::optional<std::filesystem::path> FileSystem::GetUserDataPath() {
        const auto data = Winux::Get<Winux::FileSystem>().AppData(Winux::AppDataScope::Local);
        if (data.Succeeded()) {
            return data.Value();
        }
        return std::nullopt;
    }

    std::optional<std::filesystem::path> FileSystem::GetUserConfigPath() {
        if (const auto config = ReadEnvironmentPath(Winux::Get<Winux::Environment>(), L"APPDATA")) {
            return config;
        }
        if (const auto config = ReadEnvironmentPath(Winux::Get<Winux::Environment>(), L"XDG_CONFIG_HOME")) {
            return config;
        }
        if (const auto home = GetUserHomePath()) {
            return *home / ".config";
        }
        return std::nullopt;
    }

    bool (FileSystem::CreateDirectory)(const std::filesystem::path& path) {
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

    bool (FileSystem::DeleteFile)(const std::filesystem::path& path) {
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
