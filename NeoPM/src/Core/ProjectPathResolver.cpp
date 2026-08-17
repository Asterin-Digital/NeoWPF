#include <NeoPM/Core/ProjectPathResolver.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>

namespace NeoPM::Core {
    ProjectPathResolver::ProjectPathResolver() {
    }

    std::optional<std::filesystem::path> ProjectPathResolver::ResolvePath(
        const std::optional<std::string>& explicitPath
    ) {
        auto& logger = Utils::Logger::Instance();

        if (explicitPath) {
            std::filesystem::path providedPath(*explicitPath);

            if (std::filesystem::path(providedPath).is_absolute()) {
                if (Utils::FileSystem::DirectoryExists(providedPath)) {
                    logger.Info("PathResolver", "Using explicitly provided path: " + providedPath.string());
                    return providedPath;
                }
                logger.Warning("PathResolver", "Provided path does not exist: " + providedPath.string());
                return std::nullopt;
            }

            auto cwdPath = std::filesystem::current_path() / providedPath;
            if (Utils::FileSystem::DirectoryExists(cwdPath)) {
                logger.Info("PathResolver", "Using relative path: " + cwdPath.string());
                return cwdPath;
            }

            logger.Error("PathResolver", "Could not resolve provided path: " + providedPath.string());
            return std::nullopt;
        }

        auto optimalPath = DiscoverOptimalPath();
        if (optimalPath) {
            logger.Info("PathResolver", "Using default path: " + optimalPath->string());
            return optimalPath;
        }

        logger.Error("PathResolver", "Failed to discover default project path");
        return std::nullopt;
    }

    std::optional<std::filesystem::path> ProjectPathResolver::DiscoverOptimalPath() {
        auto homeDir = Utils::FileSystem::GetUserHomePath();
        if (!homeDir) {
            return std::nullopt;
        }

        auto reposPath = homeDir.value() / "source" / "repos";
        if (Utils::FileSystem::DirectoryExists(reposPath)) {
            return reposPath;
        }

        return homeDir;
    }
}
