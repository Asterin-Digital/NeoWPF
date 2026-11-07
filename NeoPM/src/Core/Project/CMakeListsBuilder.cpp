#include <NeoPM/Core/Project/CMakeListsBuilder.h>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace NeoPM::Core::Project {
    namespace {
        std::string NormalizeCMakePath(const std::string& value) {
            auto normalized = value;
            std::replace(normalized.begin(), normalized.end(), '\\', '/');
            return normalized;
        }
    }

    CMakeListsBuilder::CMakeListsBuilder(const std::string& projectName)
        : m_projectName(projectName) {
    }

    const char* CMakeListsBuilder::Name() const noexcept {
        return "cmakelists";
    }

    bool CMakeListsBuilder::Generate(const NeoPM::Contracts::ProjectGenerationContext& context) {
        if (context.projectName.empty() || context.projectRoot.empty()) {
            return false;
        }

        m_projectName = context.projectName;
        m_sdkIncludePath = context.sdkIncludePath.value_or(std::filesystem::path{});
        m_sdkLibraryPath = context.sdkLibraryPath.value_or(std::filesystem::path{});

        const auto projectPath = context.projectRoot / "CMakeLists.txt";
        std::ofstream output(projectPath, std::ios::out | std::ios::trunc);
        if (!output) {
            return false;
        }

        output << BuildCMakeLists();
        output.close();

        const auto presetsPath = context.projectRoot / "CMakePresets.json";
        std::ofstream presetsOutput(presetsPath, std::ios::out | std::ios::trunc);
        if (!presetsOutput) {
            return false;
        }

        presetsOutput << BuildCMakePresets();
        presetsOutput.close();

        return static_cast<bool>(output) && static_cast<bool>(presetsOutput);
    }

    CMakeListsBuilder& CMakeListsBuilder::AddCompileFile(const std::string& relativePath) {
        m_compileFiles.push_back(relativePath);
        return *this;
    }

    CMakeListsBuilder& CMakeListsBuilder::AddGeneratedXamlSources() {
        m_addGeneratedXamlSources = true;
        return *this;
    }

    CMakeListsBuilder& CMakeListsBuilder::AddHeaderFile(const std::string& relativePath) {
        m_headerFiles.push_back(relativePath);
        return *this;
    }

    CMakeListsBuilder& CMakeListsBuilder::AddResourceAssetGlob() {
        m_addResourceAssetGlob = true;
        return *this;
    }

    CMakeListsBuilder& CMakeListsBuilder::AddResourceSyncModule(const std::string& modulePath) {
        m_resourceSyncModulePath = modulePath;
        return *this;
    }

    CMakeListsBuilder& CMakeListsBuilder::AddSdkLibrary(const std::filesystem::path& libPath) {
        m_sdkLibraryPath = libPath;
        return *this;
    }

    CMakeListsBuilder& CMakeListsBuilder::AddSdkIncludePath(const std::filesystem::path& includePath) {
        m_sdkIncludePath = includePath;
        return *this;
    }

    std::string CMakeListsBuilder::BuildCMakeLists() const {
        std::ostringstream ss;

        ss << "cmake_minimum_required(VERSION 3.10)\n\n";
        ss << "project(\"" << m_projectName << "\" LANGUAGES CXX)\n\n";
        ss << "set(CMAKE_CXX_STANDARD 20)\n";
        ss << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";

        ss << "set(NEOWPF_ROOT \"${CMAKE_CURRENT_SOURCE_DIR}/../NeoWPF/include\" CACHE PATH \"Path to the NeoWPF include root\")\n";
        if (!m_sdkIncludePath.empty()) {
            ss << "set(NEOWPF_ROOT \"" << NormalizeCMakePath(m_sdkIncludePath.string()) << "\" CACHE PATH \"Path to the NeoWPF include root\" FORCE)\n";
        }
        ss << "set(NEOWPF_LIBRARY \"\" CACHE FILEPATH \"Path to the NeoWPF library\")\n";
        if (!m_sdkLibraryPath.empty()) {
            ss << "set(NEOWPF_LIBRARY \"" << NormalizeCMakePath(m_sdkLibraryPath.string()) << "\" CACHE FILEPATH \"Path to the NeoWPF library\" FORCE)\n";
        }
        ss << "if(NOT EXISTS \"${NEOWPF_ROOT}/NeoWPF\")\n";
        ss << "    message(FATAL_ERROR \"NEOWPF_ROOT does not contain NeoWPF: ${NEOWPF_ROOT}\")\n";
        ss << "endif()\n\n";

        ss << "add_executable(" << m_projectName << "\n";
        for (const auto& file : m_compileFiles) {
            ss << "    \"" << NormalizeCMakePath(file) << "\"\n";
        }
        ss << ")\n\n";

        if (m_addGeneratedXamlSources) {
            ss << "file(GLOB_RECURSE GENERATED_XAML_SOURCES CONFIGURE_DEPENDS\n"
               << "    \"${CMAKE_CURRENT_SOURCE_DIR}/Generated/*.xaml.g.cpp\"\n"
               << ")\n"
               << "target_sources(" << m_projectName << " PRIVATE ${GENERATED_XAML_SOURCES})\n\n";
        }

        ss << "target_include_directories(" << m_projectName << " PRIVATE\n";
        ss << "    \"${CMAKE_CURRENT_SOURCE_DIR}/include\"\n";
        ss << "    \"${NEOWPF_ROOT}\"\n";
        ss << ")\n\n";

        if (!m_sdkLibraryPath.empty()) {
            ss << "target_link_libraries(" << m_projectName << " PRIVATE \"${NEOWPF_LIBRARY}\")\n\n";
        }

        if (m_addResourceAssetGlob) {
            ss << "file(GLOB_RECURSE RESOURCE_ASSETS CONFIGURE_DEPENDS \"${CMAKE_CURRENT_SOURCE_DIR}/Resources/Assets/**/*\")\n";
            ss << "target_sources(" << m_projectName << " PRIVATE ${RESOURCE_ASSETS})\n\n";
        }

        if (!m_resourceSyncModulePath.empty()) {
            ss << "include(\"" << NormalizeCMakePath(m_resourceSyncModulePath) << "\")\n";
            ss << "neowpf_sync_resources(" << m_projectName << ")\n";
        }

        return ss.str();
    }

    std::string CMakeListsBuilder::BuildCMakePresets() const {
        std::ostringstream ss;

        ss << "{\n";
        ss << "  \"version\": 3,\n";
        ss << "  \"configurePresets\": [\n";
        ss << "    {\n";
        ss << "      \"name\": \"windows-base\",\n";
        ss << "      \"hidden\": true,\n";
        ss << "      \"generator\": \"Ninja Multi-Config\",\n";
        ss << "      \"binaryDir\": \"${sourceDir}/out/build/${presetName}\",\n";
        ss << "      \"cacheVariables\": {\n";
        ss << "        \"CMAKE_BUILD_TYPE\": \"Debug\",\n";
        ss << "        \"CMAKE_CXX_STANDARD\": \"20\"";
        if (!m_sdkIncludePath.empty()) {
            ss << ",\n        \"NEOWPF_ROOT\": \"" << NormalizeCMakePath(m_sdkIncludePath.string()) << "\"";
        } else {
            ss << ",\n        \"NEOWPF_ROOT\": \"${sourceDir}/../NeoWPF/include\"";
        }
        if (!m_sdkLibraryPath.empty()) {
            ss << ",\n        \"NEOWPF_LIBRARY\": \"" << NormalizeCMakePath(m_sdkLibraryPath.string()) << "\"";
        }
        ss << "\n      },\n";
        ss << "      \"condition\": {\n";
        ss << "        \"type\": \"equals\",\n";
        ss << "        \"lhs\": \"${hostSystemName}\",\n";
        ss << "        \"rhs\": \"Windows\"\n";
        ss << "      }\n";
        ss << "    },\n";
        ss << "    {\n";
        ss << "      \"name\": \"linux-base\",\n";
        ss << "      \"hidden\": true,\n";
        ss << "      \"generator\": \"Ninja Multi-Config\",\n";
        ss << "      \"binaryDir\": \"${sourceDir}/out/build/${presetName}\",\n";
        ss << "      \"cacheVariables\": {\n";
        ss << "        \"CMAKE_BUILD_TYPE\": \"Debug\",\n";
        ss << "        \"CMAKE_CXX_STANDARD\": \"20\"";
        if (!m_sdkIncludePath.empty()) {
            ss << ",\n        \"NEOWPF_ROOT\": \"" << NormalizeCMakePath(m_sdkIncludePath.string()) << "\"";
        } else {
            ss << ",\n        \"NEOWPF_ROOT\": \"${sourceDir}/../NeoWPF/include\"";
        }
        if (!m_sdkLibraryPath.empty()) {
            ss << ",\n        \"NEOWPF_LIBRARY\": \"" << NormalizeCMakePath(m_sdkLibraryPath.string()) << "\"";
        }
        ss << "\n      },\n";
        ss << "      \"condition\": {\n";
        ss << "        \"type\": \"equals\",\n";
        ss << "        \"lhs\": \"${hostSystemName}\",\n";
        ss << "        \"rhs\": \"Linux\"\n";
        ss << "      }\n";
        ss << "    },\n";
        ss << "    {\n";
        ss << "      \"name\": \"windows-debug\",\n";
        ss << "      \"displayName\": \"Windows Debug\",\n";
        ss << "      \"inherits\": \"windows-base\",\n";
        ss << "      \"cacheVariables\": {\n";
        ss << "        \"CMAKE_BUILD_TYPE\": \"Debug\"\n";
        ss << "      }\n";
        ss << "    },\n";
        ss << "    {\n";
        ss << "      \"name\": \"windows-release\",\n";
        ss << "      \"displayName\": \"Windows Release\",\n";
        ss << "      \"inherits\": \"windows-base\",\n";
        ss << "      \"cacheVariables\": {\n";
        ss << "        \"CMAKE_BUILD_TYPE\": \"Release\"\n";
        ss << "      }\n";
        ss << "    },\n";
        ss << "    {\n";
        ss << "      \"name\": \"linux-debug\",\n";
        ss << "      \"displayName\": \"Linux Debug\",\n";
        ss << "      \"inherits\": \"linux-base\",\n";
        ss << "      \"cacheVariables\": {\n";
        ss << "        \"CMAKE_BUILD_TYPE\": \"Debug\"\n";
        ss << "      }\n";
        ss << "    },\n";
        ss << "    {\n";
        ss << "      \"name\": \"linux-release\",\n";
        ss << "      \"displayName\": \"Linux Release\",\n";
        ss << "      \"inherits\": \"linux-base\",\n";
        ss << "      \"cacheVariables\": {\n";
        ss << "        \"CMAKE_BUILD_TYPE\": \"Release\"\n";
        ss << "      }\n";
        ss << "    }\n";
        ss << "  ],\n";
        ss << "  \"buildPresets\": [\n";
        ss << "    {\n";
        ss << "      \"name\": \"windows-debug\",\n";
        ss << "      \"configurePreset\": \"windows-debug\",\n";
        ss << "      \"configuration\": \"Debug\"\n";
        ss << "    },\n";
        ss << "    {\n";
        ss << "      \"name\": \"windows-release\",\n";
        ss << "      \"configurePreset\": \"windows-release\",\n";
        ss << "      \"configuration\": \"Release\"\n";
        ss << "    },\n";
        ss << "    {\n";
        ss << "      \"name\": \"linux-debug\",\n";
        ss << "      \"configurePreset\": \"linux-debug\",\n";
        ss << "      \"configuration\": \"Debug\"\n";
        ss << "    },\n";
        ss << "    {\n";
        ss << "      \"name\": \"linux-release\",\n";
        ss << "      \"configurePreset\": \"linux-release\",\n";
        ss << "      \"configuration\": \"Release\"\n";
        ss << "    }\n";
        ss << "  ]\n";
        ss << "}\n";

        return ss.str();
    }
}
