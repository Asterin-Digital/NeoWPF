/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace NeoPM::Contracts {
    struct SdkPlatformConfiguration {
        std::string version;
        std::vector<std::filesystem::path> includeDirectories;
        std::filesystem::path debugLibraryPath;
        std::filesystem::path releaseLibraryPath;
    };

    struct ProjectGenerationContext {
        std::string projectName;
        std::filesystem::path projectRoot;
        std::string defaultConfiguration;
        std::map<std::string, SdkPlatformConfiguration> sdkPlatforms;
    };

    class IProjectType {
    public:
        virtual ~IProjectType() = default;

        virtual const char* Name() const noexcept = 0;
        virtual bool Generate(const ProjectGenerationContext& context) = 0;
    };
}
