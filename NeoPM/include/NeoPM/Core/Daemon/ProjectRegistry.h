#pragma once

#include <filesystem>
#include <vector>

namespace NeoPM::Core::Daemon {
    struct RegisteredProject {
        std::filesystem::path projectRoot;
        std::filesystem::path manifestPath;
        std::filesystem::path assetRoot;
        std::filesystem::path generatedSourcePath;
        std::filesystem::path generatedHeaderPath;
    };

    class ProjectRegistry {
    public:
        static std::filesystem::path GetPath();
        static std::vector<RegisteredProject> Load();
        static bool Register(const RegisteredProject& project);
    };
}