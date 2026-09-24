/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

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
