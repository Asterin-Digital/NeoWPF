#include <NeoPM/Core/Commands/NewCommand.h>
#include <NeoPM/Core/Daemon/DaemonCommand.h>
#include <NeoPM/Core/Daemon/ProjectRegistry.h>
#include <NeoPM/Core/Daemon/StartupTaskInstaller.h>
#include <NeoPM/Core/ProjectPathResolver.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>

#include <iostream>
#include <regex>
#include <vector>

namespace NeoPM::Core::Commands {

    NewCommand::NewCommand(
        const std::string& projectName,
        const std::optional<std::string>& outputPath,
        const std::string& sdkSource,
        const std::string& sdkConfiguration,
        const std::optional<std::string>& sdkIncludePath,
        const std::optional<std::string>& sdkLibraryPath
    )
        : m_projectName(projectName),
          m_outputPath(outputPath),
          m_sdkSource(sdkSource),
          m_sdkConfiguration(sdkConfiguration),
          m_sdkIncludePath(sdkIncludePath),
          m_sdkLibraryPath(sdkLibraryPath) {
    }

    int NewCommand::Execute() {
        auto& logger = Utils::Logger::Instance();

        if (!ValidateProjectName()) {
            logger.Error("NewCommand", "Invalid project name");
            return 1;
        }

        if (!ValidateSdkSelection()) {
            return 1;
        }

        auto projectLocation = ResolveProjectLocation();

        if (!projectLocation) {
            logger.Error("NewCommand", "Failed to resolve project location");
            return 1;
        }

        auto projectRoot = projectLocation.value() / m_projectName;

        if (Utils::FileSystem::DirectoryExists(projectRoot)) {
            logger.Warning(
                "NewCommand",
                "Project directory already exists; removing it before recreating: " + projectRoot.string()
            );

            if (!Utils::FileSystem::DeleteDirectory(projectRoot, true)) {
                logger.Error(
                    "NewCommand",
                    "Failed to remove existing project directory: " + projectRoot.string()
                );
                return 1;
            }
        }

        if (!CreateProjectStructure(projectRoot)) {
            logger.Error("NewCommand", "Failed to create project structure");

            Utils::FileSystem::DeleteDirectory(projectRoot, true);
            return 1;
        }

        logger.Info(
            "NewCommand",
            "Project created successfully at: " + projectRoot.string()
        );

        if (!Daemon::ProjectRegistry::Register({
            projectRoot,
            projectRoot / "Resources" / "resources.json",
            projectRoot / "Resources" / "Assets",
            projectRoot / "Generated" / "Resources.generated.cpp",
            projectRoot / "include" / m_projectName / "Generated" / "Resources.generated.h"
        })) {
            logger.Warning("NewCommand", "Project created, but its resources could not be registered with NeoPM");
        }

        if (!Daemon::StartupTaskInstaller::InstallUserStartupEntry()) {
            logger.Warning("NewCommand", "Project created, but NeoPM could not be registered to start at user logon");
        }

        if (!Daemon::DaemonCommand::StartDetached()) {
            logger.Warning("NewCommand", "Project created, but the NeoPM daemon could not be started");
        }

        std::cout
            << "Project '"
            << m_projectName
            << "' created at: "
            << projectRoot.string()
            << std::endl;

        return 0;
    }

    const char* NewCommand::GetName() const {
        return "new";
    }

    const char* NewCommand::GetDescription() const {
        return "Create a new NeoWPF project";
    }

    bool NewCommand::ValidateProjectName() const {
        if (m_projectName.empty() || m_projectName.length() > 260) {
            return false;
        }

        auto validPattern = std::regex("^[a-zA-Z][a-zA-Z0-9._-]*$");
        return std::regex_match(m_projectName, validPattern);
    }

    bool NewCommand::ValidateSdkSelection() const {
        auto& logger = Utils::Logger::Instance();

        if (m_sdkSource != "production" && m_sdkSource != "user") {
            logger.Error("NewCommand", "SDK source must be 'production' or 'user'");
            return false;
        }

        if (m_sdkConfiguration != "debug" && m_sdkConfiguration != "release") {
            logger.Error("NewCommand", "SDK configuration must be 'debug' or 'release'");
            return false;
        }

        if (m_sdkSource == "production") {
            return true;
        }

        if (!m_sdkIncludePath || !m_sdkLibraryPath) {
            logger.Error(
                "NewCommand",
                "User SDKs require --sdk-include and --sdk-lib"
            );
            return false;
        }

        auto includePath = std::filesystem::path(*m_sdkIncludePath);
        auto libraryPath = std::filesystem::path(*m_sdkLibraryPath);

        if (!std::filesystem::is_directory(includePath)) {
            logger.Error("NewCommand", "SDK include directory does not exist: " + includePath.string());
            return false;
        }

        if (!std::filesystem::is_directory(includePath / "NeoWPF") ||
            !std::filesystem::is_directory(includePath / "NeoNaml")) {
            logger.Error("NewCommand", "SDK include root must contain NeoWPF and NeoNaml folders");
            return false;
        }

        const auto containsLibrary = [](const std::filesystem::path& directory) {
            if (!std::filesystem::is_directory(directory)) {
                return false;
            }
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file() &&
                    (entry.path().extension() == ".lib" || entry.path().extension() == ".a")) {
                    return true;
                }
            }
            return false;
        };

        if (!std::filesystem::is_directory(libraryPath) ||
            !containsLibrary(libraryPath / "Debug") ||
            !containsLibrary(libraryPath / "Release")) {
            logger.Error("NewCommand", "SDK library root must contain Debug and Release libraries");
            return false;
        }

        return true;
    }

    std::optional<std::filesystem::path> NewCommand::ResolveProjectLocation() {
        auto resolver = ProjectPathResolver{};
        return resolver.ResolvePath(m_outputPath);
    }

    bool NewCommand::CreateProjectStructure(
        const std::filesystem::path& projectRoot
    ) {
        auto& logger = Utils::Logger::Instance();

        if (!Utils::FileSystem::CreateDirectories(projectRoot)) {
            logger.Error(
                "NewCommand",
                "Failed to create project root directory"
            );

            return false;
        }

        auto requiredDirs = std::vector<std::string>{
            "include",
            "Resources/Assets",
            "Generated",
            ".vs"
        };

        for (const auto& dirName : requiredDirs) {
            auto dirPath = projectRoot / dirName;

            if (!Utils::FileSystem::CreateDirectories(dirPath)) {
                logger.Error(
                    "NewCommand",
                    "Failed to create directory: " + dirPath.string()
                );

                return false;
            }
        }

        if (!Utils::FileSystem::CreateDirectories(
                projectRoot / "include" / m_projectName)) {

            logger.Error(
                "NewCommand",
                "Failed to create include app directory"
            );

            return false;
        }

        if (!GenerateMainWindow(projectRoot)) {
            logger.Error(
                "NewCommand",
                "Failed to generate MainWindow files"
            );

            return false;
        }

        if (!GenerateApp(projectRoot)) {
            logger.Error(
                "NewCommand",
                "Failed to generate App.xaml and App.xaml.cpp"
            );

            return false;
        }

        if (!GenerateAssemblyInfo(projectRoot)) {
            logger.Error(
                "NewCommand",
                "Failed to generate AssemblyInfo files"
            );

            return false;
        }

        if (!GenerateProjectFiles(projectRoot)) {
            logger.Error(
                "NewCommand",
                "Failed to generate project files"
            );

            return false;
        }

        if (!GenerateResourceManifest(projectRoot)) {
            logger.Error(
                "NewCommand",
                "Failed to generate resource manifest"
            );

            return false;
        }

        if (!GenerateResourceStubs(projectRoot)) {
            logger.Error(
                "NewCommand",
                "Failed to generate resource stubs"
            );

            return false;
        }

        return true;
    }

}