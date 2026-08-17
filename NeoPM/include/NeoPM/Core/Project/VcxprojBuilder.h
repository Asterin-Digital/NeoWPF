#pragma once

#include <NeoPM/Contracts/IProjectType.h>

#include <filesystem>
#include <string>
#include <vector>

namespace NeoPM::Core::Project {
    class VcxprojBuilder final : public NeoPM::Contracts::IProjectType {
    public:
        explicit VcxprojBuilder(const std::string& projectName);

        const char* Name() const noexcept override;
        bool Generate(const NeoPM::Contracts::ProjectGenerationContext& context) override;

        VcxprojBuilder& AddCompileFile(const std::string& relativePath);
        VcxprojBuilder& AddGeneratedXamlSources();
        VcxprojBuilder& AddHeaderFile(const std::string& relativePath);
        VcxprojBuilder& AddTargetsImport(const std::string& targetsPath);
        VcxprojBuilder& AddResourceAssetGlob();
        VcxprojBuilder& AddNeoWPFToolsDir();
        VcxprojBuilder& AddSdkLibrary(const std::filesystem::path& libPath);
        VcxprojBuilder& AddSdkIncludePath(const std::filesystem::path& includePath);

        bool GenerateGuid();

        const std::string& GetProjectName() const noexcept;
        const std::string& GetProjectGuid() const noexcept;

        std::string BuildVcxproj() const;
        std::string BuildVcxprojFilters() const;
        std::string BuildSlnx() const;

    private:
        std::string m_projectName;
        std::string m_projectGuid;

        std::vector<std::string> m_compileFiles;
        std::vector<std::string> m_headerFiles;
        bool m_addGeneratedXamlSources = false;
        std::string m_targetsImportPath;
        bool m_addResourceAssetGlob = false;
        bool m_addNeoWPFToolsDir = false;
        std::filesystem::path m_sdkLibraryPath;
        std::filesystem::path m_sdkIncludePath;
    };
}