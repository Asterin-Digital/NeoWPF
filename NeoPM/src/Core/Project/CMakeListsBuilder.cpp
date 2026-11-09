#include <NeoPM/Core/Project/CMakeListsBuilder.h>

#include <nlohmann/json.hpp>

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

        std::string JoinCMakePaths(const std::vector<std::filesystem::path>& paths) {
            std::string result;
            for (const auto& path : paths) {
                if (!result.empty()) result += ';';
                result += NormalizeCMakePath(path.string());
            }
            return result;
        }

        const NeoPM::Contracts::SdkPlatformConfiguration* FindPlatformConfiguration(
            const std::map<std::string, NeoPM::Contracts::SdkPlatformConfiguration>& platforms,
            const std::string& platform)
        {
            const auto found = platforms.find(platform);
            return found == platforms.end() ? nullptr : &found->second;
        }

        void WritePlatformDefaults(
            std::ostringstream& stream,
            const NeoPM::Contracts::SdkPlatformConfiguration* configuration)
        {
            if (!configuration) {
                stream << "    if(NOT DEFINED NEOWPF_INCLUDE_DIRECTORIES)\n"
                       << "        set(NEOWPF_INCLUDE_DIRECTORIES \"${CMAKE_CURRENT_SOURCE_DIR}/../NeoWPF/include\")\n"
                       << "    endif()\n"
                       << "    if(NOT DEFINED NEOWPF_SDK_VERSION)\n"
                       << "        set(NEOWPF_SDK_VERSION \"\")\n"
                       << "    endif()\n"
                       << "    if(NOT DEFINED NEOWPF_LIBRARY_DEBUG)\n"
                       << "        set(NEOWPF_LIBRARY_DEBUG \"\")\n"
                       << "    endif()\n"
                       << "    if(NOT DEFINED NEOWPF_LIBRARY_RELEASE)\n"
                       << "        set(NEOWPF_LIBRARY_RELEASE \"\")\n"
                       << "    endif()\n";
                return;
            }

            stream << "    if(NOT DEFINED NEOWPF_INCLUDE_DIRECTORIES)\n"
                   << "        set(NEOWPF_INCLUDE_DIRECTORIES \""
                   << NormalizeCMakePath(JoinCMakePaths(configuration->includeDirectories)) << "\")\n"
                   << "    endif()\n"
                   << "    if(NOT DEFINED NEOWPF_SDK_VERSION)\n"
                   << "        set(NEOWPF_SDK_VERSION \"" << configuration->version << "\")\n"
                   << "    endif()\n"
                   << "    if(NOT DEFINED NEOWPF_LIBRARY_DEBUG)\n"
                   << "        set(NEOWPF_LIBRARY_DEBUG \""
                   << NormalizeCMakePath(configuration->debugLibraryPath.string()) << "\")\n"
                   << "    endif()\n"
                   << "    if(NOT DEFINED NEOWPF_LIBRARY_RELEASE)\n"
                   << "        set(NEOWPF_LIBRARY_RELEASE \""
                   << NormalizeCMakePath(configuration->releaseLibraryPath.string()) << "\")\n"
                   << "    endif()\n";
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
        m_defaultConfiguration = context.defaultConfiguration;
        m_sdkPlatforms = context.sdkPlatforms;

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

    CMakeListsBuilder& CMakeListsBuilder::SetSdkPlatforms(
        const std::map<std::string, NeoPM::Contracts::SdkPlatformConfiguration>& sdkPlatforms) {
        m_sdkPlatforms = sdkPlatforms;
        return *this;
    }

    std::string CMakeListsBuilder::BuildCMakeLists() const {
        std::ostringstream ss;

        ss << "cmake_minimum_required(VERSION 3.10)\n\n";
        ss << "project(\"" << m_projectName << "\" LANGUAGES CXX)\n\n";
        ss << "set(CMAKE_CXX_STANDARD 20)\n";
        ss << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
        if (m_defaultConfiguration == "debug" || m_defaultConfiguration == "release") {
            ss << "if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)\n"
               << "    set(CMAKE_BUILD_TYPE \""
               << (m_defaultConfiguration == "debug" ? "Debug" : "Release")
               << "\" CACHE STRING \"Build configuration\" FORCE)\n"
               << "endif()\n\n";
        }

        if (m_sdkPlatforms.empty()) {
            ss << "set(NEOWPF_INCLUDE_DIRECTORIES \"${CMAKE_CURRENT_SOURCE_DIR}/../NeoWPF/include\")\n";
        } else {
            ss << "if(WIN32)\n";
            WritePlatformDefaults(ss, FindPlatformConfiguration(m_sdkPlatforms, "Windows"));
            ss << "elseif(UNIX AND NOT APPLE)\n";
            WritePlatformDefaults(ss, FindPlatformConfiguration(m_sdkPlatforms, "Linux"));
            ss << "else()\n";
            ss << "    message(FATAL_ERROR \"No pinned NeoWPF SDK mapping exists for this platform\")\n";
            ss << "endif()\n";
        }
        ss << "\n";
        ss << "set(_NEOWPF_HAS_HEADERS FALSE)\n";
        ss << "set(_NEONAML_HAS_HEADERS FALSE)\n";
        ss << "foreach(_NEOWPF_INCLUDE_DIRECTORY IN LISTS NEOWPF_INCLUDE_DIRECTORIES)\n";
        ss << "    if(EXISTS \"${_NEOWPF_INCLUDE_DIRECTORY}/NeoWPF\")\n";
        ss << "        set(_NEOWPF_HAS_HEADERS TRUE)\n";
        ss << "    endif()\n";
        ss << "    if(EXISTS \"${_NEOWPF_INCLUDE_DIRECTORY}/NeoNaml\")\n";
        ss << "        set(_NEONAML_HAS_HEADERS TRUE)\n";
        ss << "    endif()\n";
        ss << "endforeach()\n";
        ss << "if(NOT _NEOWPF_HAS_HEADERS OR NOT _NEONAML_HAS_HEADERS)\n";
        ss << "    message(FATAL_ERROR \"NEOWPF_INCLUDE_DIRECTORIES must contain NeoWPF and NeoNaml headers\")\n";
        ss << "endif()\n\n";
        if (!m_sdkPlatforms.empty()) {
            ss << "if(NOT EXISTS \"${NEOWPF_LIBRARY_DEBUG}\" OR NOT EXISTS \"${NEOWPF_LIBRARY_RELEASE}\")\n";
            ss << "    message(FATAL_ERROR \"Pinned NeoWPF SDK libraries are missing for ${NEOWPF_SDK_VERSION}\")\n";
            ss << "endif()\n\n";
        }

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
        ss << "    ${NEOWPF_INCLUDE_DIRECTORIES}\n";
        ss << ")\n\n";

        if (!m_sdkPlatforms.empty()) {
            ss << "target_link_libraries(" << m_projectName << " PRIVATE\n"
               << "    \"$<$<CONFIG:Debug>:${NEOWPF_LIBRARY_DEBUG}>\"\n"
               << "    \"$<$<NOT:$<CONFIG:Debug>>:${NEOWPF_LIBRARY_RELEASE}>\"\n"
               << ")\n\n";
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
        using json = nlohmann::json;

        json presets = {
            {"version", 3},
            {"configurePresets", json::array()},
            {"buildPresets", json::array()}};

        const auto appendPlatformPresets = [this, &presets](
            const std::string& platform,
            const std::string& presetName) {
            const auto* sdk = FindPlatformConfiguration(m_sdkPlatforms, platform);
            if (!sdk && !m_sdkPlatforms.empty()) return;

            json cacheVariables = {
                {"CMAKE_BUILD_TYPE", "Debug"},
                {"CMAKE_CXX_STANDARD", "20"}};
            if (sdk) {
                cacheVariables["NEOWPF_INCLUDE_DIRECTORIES"] = JoinCMakePaths(sdk->includeDirectories);
                cacheVariables["NEOWPF_SDK_VERSION"] = sdk->version;
                cacheVariables["NEOWPF_LIBRARY_DEBUG"] = NormalizeCMakePath(sdk->debugLibraryPath.string());
                cacheVariables["NEOWPF_LIBRARY_RELEASE"] = NormalizeCMakePath(sdk->releaseLibraryPath.string());
            } else {
                cacheVariables["NEOWPF_INCLUDE_DIRECTORIES"] = "${sourceDir}/../NeoWPF/include";
            }

            presets["configurePresets"].push_back({
                {"name", presetName + "-base"},
                {"hidden", true},
                {"generator", "Ninja Multi-Config"},
                {"binaryDir", "${sourceDir}/out/build/${presetName}"},
                {"cacheVariables", cacheVariables},
                {"condition", {
                    {"type", "equals"},
                    {"lhs", "${hostSystemName}"},
                    {"rhs", platform}}}});

            for (const auto& [configuration, displayName] : {
                     std::pair<std::string, std::string>{"debug", "Debug"},
                     {"release", "Release"}}) {
                presets["configurePresets"].push_back({
                    {"name", presetName + "-" + configuration},
                    {"displayName", platform + " " + displayName},
                    {"inherits", presetName + "-base"},
                    {"cacheVariables", {{"CMAKE_BUILD_TYPE", displayName}}}});
                presets["buildPresets"].push_back({
                    {"name", presetName + "-" + configuration},
                    {"configurePreset", presetName + "-" + configuration},
                    {"configuration", displayName}});
            }
        };

        appendPlatformPresets("Windows", "windows");
        appendPlatformPresets("Linux", "linux");
        return presets.dump(2) + "\n";
    }
}
