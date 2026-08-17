#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace NeoPM::Core {
    class ProjectPathResolver {
    public:
        explicit ProjectPathResolver();

        std::optional<std::filesystem::path> ResolvePath(
            const std::optional<std::string>& explicitPath = std::nullopt
        );

    private:
        std::optional<std::filesystem::path> DiscoverOptimalPath();
    };
}
