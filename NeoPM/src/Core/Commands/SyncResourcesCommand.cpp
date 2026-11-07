#include <NeoPM/Core/Commands/SyncResourcesCommand.h>
#include <NeoPM/Core/Daemon/DaemonCommand.h>
#include <NeoPM/Resources/ResourceManifest.h>
#include <NeoPM/Resources/ResourceValidator.h>
#include <NeoPM/Resources/ResourceCodegen.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>
#include <NeoPM/Utils/NamedMutex.h>
#include <filesystem>
#include <iostream>

namespace NeoPM::Core::Commands {
    using namespace Resources;

    namespace {
        class ResourceGenerationLock {
        public:
            ResourceGenerationLock()
                : m_mutex("NeoWPF.NeoPM.ResourceGeneration"), m_acquired(m_mutex.Acquire()) {}

            bool Acquired() const {
                return m_acquired;
            }

        private:
            Utils::NamedMutex m_mutex;
            bool m_acquired = false;
        };
    }

    SyncResourcesCommand::SyncResourcesCommand(
        const std::string& manifestPath,
        const std::optional<std::string>& outputDir,
        const std::optional<std::string>& projectName,
        const std::optional<std::string>& copyOutputDir
    )
        : m_manifestPath(manifestPath), m_outputDir(outputDir), m_projectName(projectName), m_copyOutputDir(copyOutputDir) {
    }

    int SyncResourcesCommand::Execute() {
        auto& logger = Utils::Logger::Instance();
        ResourceGenerationLock generationLock;
        if (!generationLock.Acquired()) {
            logger.Error("SyncResourcesCommand", "Unable to acquire the resource generation lock");
            return 1;
        }


        auto manifestPath = std::filesystem::path(m_manifestPath);

        if (!Utils::FileSystem::FileExists(manifestPath)) {
            logger.Error("SyncResourcesCommand", "Manifest file not found: " + m_manifestPath);
            return 1;
        }


        auto manifest = ResourceManifest::Load(manifestPath);
        if (!manifest) {
            logger.Error("SyncResourcesCommand", "Failed to load manifest");
            return 1;
        }

        logger.Info("SyncResourcesCommand", "Loaded manifest with " + std::to_string(manifest->GetEntries().size()) + " resources");


        auto projectRoot = manifestPath.parent_path().parent_path();

        if (!Utils::FileSystem::DirectoryExists(projectRoot)) {
            logger.Error("SyncResourcesCommand", "Invalid project root: " + projectRoot.string());
            return 1;
        }

        if (!Daemon::DaemonCommand::EnsureRunning()) {
            logger.Warning("SyncResourcesCommand", "Unable to start or connect to the NeoPM daemon; continuing with direct synchronization");
        }

        if (!m_projectName.has_value()) {
            m_projectName = ResolveProjectName(projectRoot);
        }

        auto errors = std::vector<ValidationError>{};
        auto validator = ResourceValidator{};
        if (!validator.Validate(manifest.value(), projectRoot, errors)) {
            for (const auto& error : errors) {
                if (error.isWarning) {
                    logger.Warning("SyncResourcesCommand", error.message);
                } else {
                    logger.Error("SyncResourcesCommand", error.message);
                }
            }
            logger.Error("SyncResourcesCommand", "Manifest validation failed");
            return 1;
        }


        for (const auto& error : errors) {
            if (error.isWarning) {
                logger.Warning("SyncResourcesCommand", error.message);
            }
        }


        auto outputPath = m_outputDir
            ? std::filesystem::path(m_outputDir.value())
            : projectRoot / "Generated";

        std::optional<std::filesystem::path> copyOutputPath;
        if (m_copyOutputDir.has_value()) {
            copyOutputPath = std::filesystem::path(m_copyOutputDir.value());
        }

        ResourceCodegen codegen;
        if (!codegen.Generate(manifest.value(), projectRoot, m_projectName.value(), outputPath, copyOutputPath)) {
            logger.Error("SyncResourcesCommand", "Code generation failed");
            return 1;
        }

        logger.Info("SyncResourcesCommand", "Resources synchronized successfully");

        std::cout << "Resources synchronized to: " << outputPath.string() << std::endl;

        return 0;
    }

    const char* SyncResourcesCommand::GetName() const {
        return "sync-resources";
    }

    const char* SyncResourcesCommand::GetDescription() const {
        return "Synchronize and generate resource code from manifest";
    }

    std::string SyncResourcesCommand::ResolveProjectName(const std::filesystem::path& projectRoot) const {
        return projectRoot.filename().string();
    }
}
