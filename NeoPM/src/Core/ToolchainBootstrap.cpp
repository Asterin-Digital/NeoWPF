#include <NeoPM/Core/ToolchainBootstrap.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>

#include <filesystem>
#include <cctype>
#include <fstream>
#include <string>

namespace NeoPM::Core {
    namespace {
        std::string XmlEscape(const std::string& value) {
            auto escaped = std::string{};
            escaped.reserve(value.size());

            for (const auto ch : value) {
                switch (ch) {
                case '&':
                    escaped += "&amp;";
                    break;
                case '<':
                    escaped += "&lt;";
                    break;
                case '>':
                    escaped += "&gt;";
                    break;
                case '"':
                    escaped += "&quot;";
                    break;
                case '\'':
                    escaped += "&apos;";
                    break;
                default:
                    escaped += ch;
                    break;
                }
            }

            return escaped;
        }
    }

    bool ToolchainBootstrap::EnsureInstalled() {
        auto& logger = Utils::Logger::Instance();

        auto toolDirectory = GetToolDirectory();

        if (!Utils::FileSystem::DirectoryExists(toolDirectory)) {
            if (!Utils::FileSystem::CreateDirectories(toolDirectory)) {
                logger.Error("ToolchainBootstrap", "Failed to create LocalAppData NeoWPF directory");
                return false;
            }
        }

        auto currentExecutable = Utils::FileSystem::GetExecutablePath();
        auto targetExecutable = GetToolExecutablePath();

        if (currentExecutable.has_value()) {
            try {
                std::error_code removeError;
                const auto legacyTargetExecutable = GetToolDirectory() / "NeoPM";
                if (std::filesystem::exists(legacyTargetExecutable)) {
                    std::filesystem::remove(legacyTargetExecutable, removeError);
                }
                if (std::filesystem::exists(targetExecutable)) {
                    std::filesystem::remove(targetExecutable, removeError);
                }

                std::filesystem::copy_file(
                    currentExecutable.value(),
                    targetExecutable,
                    std::filesystem::copy_options::overwrite_existing
                );
            }
            catch (const std::filesystem::filesystem_error&) {
                logger.Debug("ToolchainBootstrap", "NeoPM bootstrap refresh is in progress or the executable is in use; continuing with the current copy");
            }
        }

        auto targetsPath = GetTargetsFilePath();
        if (std::filesystem::exists(targetsPath)) {
            std::filesystem::remove(targetsPath);
        }

        auto cmakeModulePath = GetCMakeModulePath();
        if (std::filesystem::exists(cmakeModulePath)) {
            std::filesystem::remove(cmakeModulePath);
        }

        auto targetsXml = BuildTargetsXml(targetExecutable);
        auto cmakeModule = BuildResourceSyncCMakeModule(targetExecutable);

        auto file = std::ofstream(targetsPath, std::ios::out | std::ios::trunc);
        if (!file) {
            logger.Error("ToolchainBootstrap", "Failed to write NeoWPF.Resources.targets");
            return false;
        }

        file << targetsXml;
        file.close();

        auto cmakeFile = std::ofstream(cmakeModulePath, std::ios::out | std::ios::trunc);
        if (!cmakeFile) {
            logger.Error("ToolchainBootstrap", "Failed to write NeoWPFResources.cmake");
            return false;
        }

        cmakeFile << cmakeModule;
        cmakeFile.close();

        logger.Info("ToolchainBootstrap", "NeoWPF toolchain bootstrap installed at: " + targetsPath.string());
        return true;
    }

    std::filesystem::path ToolchainBootstrap::GetLocalAppDataDirectory() {
        return Utils::FileSystem::GetUserDataPath().value_or(std::filesystem::temp_directory_path());
    }

    std::filesystem::path ToolchainBootstrap::GetToolDirectory() {
        return GetLocalAppDataDirectory() / "NeoWPF";
    }

    std::filesystem::path ToolchainBootstrap::GetTargetsFilePath() {
        return GetToolDirectory() / "NeoWPF.Resources.targets";
    }

    std::filesystem::path ToolchainBootstrap::GetCMakeModulePath() {
        return GetToolDirectory() / "NeoWPFResources.cmake";
    }

    std::filesystem::path ToolchainBootstrap::GetToolExecutablePath() {
#if defined(_WIN32)
        return GetToolDirectory() / "NeoPM.exe";
#else
        return GetToolDirectory() / "NeoPM";
#endif
    }

    std::string ToolchainBootstrap::BuildTargetsXml(const std::filesystem::path& toolExecutablePath) {
        auto toolPath = XmlEscape(toolExecutablePath.string());

        return std::string{
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<Project ToolsVersion=\"Current\">\n"
            "  <PropertyGroup>\n"
            "    <NeoWPFToolsDir>$(LocalAppData)\\NeoWPF\\</NeoWPFToolsDir>\n"
            "    <NeoWPFToolExecutable>$(NeoWPFToolsDir)NeoPM.exe</NeoWPFToolExecutable>\n"
            "  </PropertyGroup>\n"
            "\n"
            "  <ItemGroup>\n"
            "    <ResourceManifest Include=\"$(ProjectDir)Resources\\resources.json\">\n"
            "      <ExcludedFromBuild>true</ExcludedFromBuild>\n"
            "    </ResourceManifest>\n"
            "  </ItemGroup>\n"
            "\n"
            "  <Target Name=\"NeoWPF_SyncResources\" BeforeTargets=\"ClCompile\">\n"
            "    <Exec Command=\"&quot;$(NeoWPFToolExecutable)&quot; sync-resources &quot;$(ProjectDir)Resources\\resources.json&quot; --output &quot;$(ProjectDir)Generated&quot; --copy-to &quot;$(TargetDir)&quot;\" />\n"
            "  </Target>\n"
            "\n"
            "</Project>\n"
        };
    }

    std::string ToolchainBootstrap::BuildResourceSyncCMakeModule(const std::filesystem::path& toolExecutablePath) {
        auto windowsExecutable = toolExecutablePath.generic_string();
        std::string wslExecutable;
        if (windowsExecutable.size() >= 3 && windowsExecutable[1] == ':' && windowsExecutable[2] == '/') {
            wslExecutable = "/mnt/";
            wslExecutable += static_cast<char>(std::tolower(static_cast<unsigned char>(windowsExecutable[0])));
            wslExecutable += windowsExecutable.substr(2);
        }

        return std::string{
            "function(neowpf_sync_resources TARGET_NAME)\n"
            "  if(NOT NEOPM_EXECUTABLE)\n"
            "    if(UNIX AND EXISTS \"/proc/version\")\n"
            "      file(READ \"/proc/version\" _neopm_proc_version)\n"
            "      if(_neopm_proc_version MATCHES \"[Mm]icrosoft\")\n"
            "        set(NEOPM_EXECUTABLE \"" + wslExecutable + "\")\n"
            "      endif()\n"
            "    endif()\n"
            "    if(NOT NEOPM_EXECUTABLE AND WIN32)\n"
            "      set(NEOPM_EXECUTABLE \"$ENV{LOCALAPPDATA}/NeoWPF/NeoPM.exe\")\n"
            "    elseif(NOT NEOPM_EXECUTABLE)\n"
            "      if(DEFINED ENV{XDG_DATA_HOME} AND NOT \"$ENV{XDG_DATA_HOME}\" STREQUAL \"\")\n"
            "        set(NEOPM_EXECUTABLE \"$ENV{XDG_DATA_HOME}/NeoWPF/NeoPM\")\n"
            "      else()\n"
            "        set(NEOPM_EXECUTABLE \"$ENV{HOME}/.local/share/NeoWPF/NeoPM\")\n"
            "      endif()\n"
            "    endif()\n"
            "  endif()\n"
            "  if(NOT NEOPM_EXECUTABLE OR NOT EXISTS \"${NEOPM_EXECUTABLE}\")\n"
            "    message(FATAL_ERROR \"NeoPM executable is required for resource synchronization at: ${NEOPM_EXECUTABLE}\")\n"
            "  endif()\n"
            "  set(_manifest \"${CMAKE_CURRENT_SOURCE_DIR}/Resources/resources.json\")\n"
            "  file(GLOB_RECURSE _xaml_sources CONFIGURE_DEPENDS\n"
            "    \"${CMAKE_CURRENT_SOURCE_DIR}/*.xaml\"\n"
            "  )\n"
            "\n"
            "  add_custom_target(${TARGET_NAME}_NeoWPFCodegen ALL\n"
            "    COMMAND \"${NEOPM_EXECUTABLE}\" sync-resources \"${_manifest}\" --output \"${CMAKE_CURRENT_SOURCE_DIR}/Generated\" --copy-to \"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>\"\n"
            "    DEPENDS \"${_manifest}\" ${_xaml_sources}\n"
            "    COMMENT \"Syncing NeoWPF resources for ${TARGET_NAME}\"\n"
            "    VERBATIM\n"
            "  )\n"
            "  add_dependencies(${TARGET_NAME} ${TARGET_NAME}_NeoWPFCodegen)\n"
            "endfunction()\n"
        };
    }
}
