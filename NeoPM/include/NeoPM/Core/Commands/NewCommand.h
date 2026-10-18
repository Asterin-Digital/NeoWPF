/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Core/ICommand.h>
#include <optional>
#include <string>
#include <filesystem>

namespace NeoPM::Core::Commands {
    class NewCommand : public ICommand {
    public:
        explicit NewCommand(
            const std::string& projectName,
            const std::optional<std::string>& outputPath = std::nullopt,
            const std::string& projectType = "vcxproj",
            const std::string& sdkSource = "production",
            const std::string& sdkConfiguration = "debug",
            const std::optional<std::string>& sdkIncludePath = std::nullopt,
            const std::optional<std::string>& sdkLibraryPath = std::nullopt
        );

        int Execute() override;
        const char* GetName() const override;
        const char* GetDescription() const override;

    private:
        std::string m_projectName;
        std::optional<std::string> m_outputPath;
        std::string m_projectType;
        std::string m_sdkSource;
        std::string m_sdkConfiguration;
        std::optional<std::string> m_sdkIncludePath;
        std::optional<std::string> m_sdkLibraryPath;

        bool ValidateProjectName() const;
        bool ValidateProjectType() const;
        bool ValidateSdkSelection() const;
        std::optional<std::filesystem::path> ResolveProjectLocation();
        bool CreateProjectStructure(const std::filesystem::path& projectRoot);
        
        bool GenerateMainWindow(const std::filesystem::path& projectRoot);
        bool GenerateApp(const std::filesystem::path& projectRoot);
        bool GenerateAssemblyInfo(const std::filesystem::path& projectRoot);
        bool GenerateProjectFiles(const std::filesystem::path& projectRoot);
        bool GenerateResourceManifest(const std::filesystem::path& projectRoot);
        bool GenerateResourceStubs(const std::filesystem::path& projectRoot);
    };
}

