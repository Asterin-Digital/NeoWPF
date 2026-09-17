#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace NeoPM::Contracts {
    struct ProjectGenerationContext {
        std::string projectName;
        std::filesystem::path projectRoot;
        std::optional<std::filesystem::path> sdkIncludePath;
        std::optional<std::filesystem::path> sdkLibraryPath;
    };

    class IProjectType {
    public:
        virtual ~IProjectType() = default;

        virtual const char* Name() const noexcept = 0;
        virtual bool Generate(const ProjectGenerationContext& context) = 0;
    };
}
