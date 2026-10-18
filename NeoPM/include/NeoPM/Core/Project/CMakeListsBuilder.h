/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Contracts/IProjectType.h>

#include <filesystem>
#include <string>
#include <vector>

namespace NeoPM::Core::Project {
    class CMakeListsBuilder final : public NeoPM::Contracts::IProjectType {
    public:
        explicit CMakeListsBuilder(const std::string& projectName);

        const char* Name() const noexcept override;
        bool Generate(const NeoPM::Contracts::ProjectGenerationContext& context) override;

        CMakeListsBuilder& AddCompileFile(const std::string& relativePath);
        CMakeListsBuilder& AddGeneratedXamlSources();
        CMakeListsBuilder& AddHeaderFile(const std::string& relativePath);
        CMakeListsBuilder& AddResourceAssetGlob();
        CMakeListsBuilder& AddResourceSyncModule(const std::string& modulePath);
        CMakeListsBuilder& AddSdkLibrary(const std::filesystem::path& libPath);
        CMakeListsBuilder& AddSdkIncludePath(const std::filesystem::path& includePath);

        std::string BuildCMakeLists() const;
        std::string BuildCMakePresets() const;

    private:
        std::string m_projectName;
        std::vector<std::string> m_compileFiles;
        std::vector<std::string> m_headerFiles;
        bool m_addGeneratedXamlSources = false;
        bool m_addResourceAssetGlob = false;
        std::string m_resourceSyncModulePath;
        std::filesystem::path m_sdkLibraryPath;
        std::filesystem::path m_sdkIncludePath;
    };
}
