#include <NeoPM/Core/Commands/RepairCommand.h>

#include <NeoPM/Core/Daemon/ProjectRegistry.h>
#include <NeoPM/Core/ToolchainBootstrap.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>
#include <NeoPM/Utils/Process.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace NeoPM::Core::Commands {
namespace {

bool RepairProjectFile(const std::filesystem::path& projectFile)
{
    std::ifstream input(projectFile, std::ios::binary);
    if (!input) {
        return false;
    }

    std::ostringstream contents;
    contents << input.rdbuf();
    auto text = contents.str();

    const std::string targetsImport =
        "  <Import Project=\"$(LocalAppData)\\NeoWPF\\NeoWPF.Resources.targets\" />";
    if (text.find(targetsImport) == std::string::npos) {
        const std::string cppTargetsImport =
            "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />";
        const auto position = text.find(cppTargetsImport);
        if (position == std::string::npos) {
            return false;
        }
        text.insert(position, targetsImport + "\n");
    }

    const std::string resourceManifest =
        "    <ResourceManifest Include=\"$(ProjectDir)Resources\\resources.json\">\n"
        "      <ExcludedFromBuild>true</ExcludedFromBuild>\n"
        "    </ResourceManifest>";
    if (text.find("<ResourceManifest Include=\"$(ProjectDir)Resources\\resources.json\">") == std::string::npos) {
        const auto itemGroup = text.find("  <ItemGroup>");
        if (itemGroup == std::string::npos) {
            return false;
        }
        const auto insertPosition = itemGroup + std::string("  <ItemGroup>").size();
        text.insert(insertPosition, "\n" + resourceManifest);
    }

    std::ofstream output(projectFile, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }
    output << text;
    return true;
}

bool RepairCMakeProject(const std::filesystem::path& projectRoot)
{
    const auto cmakeListsPath = projectRoot / "CMakeLists.txt";
    std::ifstream input(cmakeListsPath, std::ios::binary);
    if (!input) {
        return false;
    }

    std::ostringstream contents;
    contents << input.rdbuf();
    auto text = contents.str();
    const auto targetName = projectRoot.filename().string();

    const auto modulePath = projectRoot / "cmake" / "NeoWPFResources.cmake";
    std::error_code error;
    std::filesystem::create_directories(modulePath.parent_path(), error);
    if (error) {
        return false;
    }

    std::ofstream module(modulePath, std::ios::out | std::ios::trunc);
    if (!module) {
        return false;
    }
    module << ToolchainBootstrap::BuildResourceSyncCMakeModule(
        ToolchainBootstrap::GetToolExecutablePath());
    module.close();

    if (text.find("neowpf_sync_resources(") == std::string::npos) {
        text += "\ninclude(\"${CMAKE_CURRENT_SOURCE_DIR}/cmake/NeoWPFResources.cmake\")\n";
        text += "neowpf_sync_resources(" + targetName + ")\n";

        std::ofstream output(cmakeListsPath, std::ios::binary | std::ios::trunc);
        if (!output) {
            return false;
        }
        output << text;
    }

    return true;
}

void TerminateOtherNeoPMProcesses()
{
    Utils::Process::TerminateByName("NeoPM");
}

bool ReplaceInstalledNeoPM()
{
    const auto currentExecutable = Utils::FileSystem::GetExecutablePath();
    if (!currentExecutable) {
        return false;
    }

    const auto installedExecutable = ToolchainBootstrap::GetToolExecutablePath();
    std::error_code error;
    if (std::filesystem::equivalent(*currentExecutable, installedExecutable, error)) {
        return true;
    }

    error.clear();
    std::filesystem::create_directories(installedExecutable.parent_path(), error);
    if (error) {
        return false;
    }

    const auto temporaryExecutable = installedExecutable.string() + ".repair";
    std::filesystem::remove(temporaryExecutable, error);
    error.clear();
    std::filesystem::copy_file(
        *currentExecutable, temporaryExecutable,
        std::filesystem::copy_options::overwrite_existing, error);
    if (error) {
        return false;
    }

    std::filesystem::remove(installedExecutable, error);
    error.clear();
    std::filesystem::rename(temporaryExecutable, installedExecutable, error);
    if (error) {
        std::filesystem::remove(temporaryExecutable);
        return false;
    }
    return true;
}

bool StartInstalledNeoPM()
{
    const auto executablePath = ToolchainBootstrap::GetToolExecutablePath();
    return Utils::Process::StartDetached(executablePath, "daemon");
}

}

RepairCommand::RepairCommand(std::filesystem::path projectPath)
    : m_projectPath(std::move(projectPath))
{
}

int RepairCommand::Execute()
{
    auto& logger = Utils::Logger::Instance();
    TerminateOtherNeoPMProcesses();
    std::error_code error;
    m_projectPath = std::filesystem::absolute(m_projectPath, error);
    if (error || !std::filesystem::is_directory(m_projectPath)) {
        logger.Error("RepairCommand", "Project directory was not found: " + m_projectPath.string());
        return 1;
    }

    const auto manifest = m_projectPath / "Resources" / "resources.json";
    if (!std::filesystem::is_regular_file(manifest)) {
        logger.Error("RepairCommand", "Project has no Resources/resources.json: " + m_projectPath.string());
        return 1;
    }

    if (!ReplaceInstalledNeoPM()) {
        logger.Error("RepairCommand", "Failed to replace the installed NeoPM executable");
        return 1;
    }

    if (!ToolchainBootstrap::EnsureInstalled()) {
        logger.Error("RepairCommand", "Failed to refresh NeoWPF build targets");
        return 1;
    }

    bool repaired = RepairCMakeProject(m_projectPath);
    for (const auto& entry : std::filesystem::directory_iterator(m_projectPath, error)) {
        if (error) {
            break;
        }
        if (entry.path().extension() == ".vcxproj" && RepairProjectFile(entry.path())) {
            repaired = true;
        }
    }

    if (error || !repaired) {
        logger.Error("RepairCommand", "No repairable CMakeLists.txt or .vcxproj file found in: " + m_projectPath.string());
        return 1;
    }

    if (!Daemon::ProjectRegistry::Register({
            m_projectPath,
            manifest,
            m_projectPath / "Resources" / "Assets",
            m_projectPath / "Generated" / "Resources.generated.cpp",
            m_projectPath / "include" / m_projectPath.filename() / "Generated" / "Resources.generated.h"})) {
        logger.Error("RepairCommand", "Failed to register project with NeoPM daemon");
        return 1;
    }

    if (!StartInstalledNeoPM()) {
        logger.Error("RepairCommand", "Failed to start the installed NeoPM daemon");
        return 1;
    }

    logger.Info("RepairCommand", "Done");
    return 0;
}

const char* RepairCommand::GetName() const { return "repair"; }
const char* RepairCommand::GetDescription() const { return "Repair NeoWPF project tooling "; }

}
