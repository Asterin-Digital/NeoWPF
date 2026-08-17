#include <NeoPM/Core/Daemon/DaemonCommand.h>
#include <NeoPM/Core/Daemon/ProjectRegistry.h>
#include <NeoPM/Core/Commands/SyncResourcesCommand.h>
#include <NeoPM/Core/ToolchainBootstrap.h>
#include <NeoPM/Resources/ResourceManifest.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>
#include <NeoPM/Utils/NamedMutex.h>
#include <NeoPM/Utils/Process.h>

#include <chrono>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace NeoPM::Core::Daemon {
    namespace {
        struct FileState {
            std::filesystem::file_time_type modified;
            std::uintmax_t size = 0;

            bool operator==(const FileState& other) const {
                return modified == other.modified && size == other.size;
            }
        };

        using ProjectState = std::map<std::filesystem::path, FileState>;

        std::vector<Resources::XamlResourceEntry> DiscoverXamlFiles(
            const std::filesystem::path& projectRoot) {
            std::vector<Resources::XamlResourceEntry> entries;
            std::error_code error;
            for (std::filesystem::recursive_directory_iterator iterator(
                     projectRoot,
                     std::filesystem::directory_options::skip_permission_denied,
                     error), end;
                 iterator != end && !error;
                 iterator.increment(error)) {
                if (iterator->is_directory(error)) {
                    continue;
                }
                if (!iterator->is_regular_file(error) || iterator->path().extension() != ".xaml") {
                    continue;
                }
                const auto relativePath = std::filesystem::relative(iterator->path(), projectRoot, error);
                if (error) {
                    continue;
                }
                auto key = relativePath;
                key.replace_extension();
                entries.push_back({key.generic_string(), relativePath.generic_string()});
            }
            std::sort(entries.begin(), entries.end(), [](const auto& left, const auto& right) {
                return left.path < right.path;
            });
            return entries;
        }

        void SynchronizeXamlManifest(const RegisteredProject& project) {
            auto manifest = Resources::ResourceManifest::Load(project.manifestPath);
            if (!manifest) {
                return;
            }
            const auto discovered = DiscoverXamlFiles(project.projectRoot);
            if (manifest->GetXamlEntries() == discovered) {
                return;
            }
            manifest->SetXamlEntries(discovered);
            if (!manifest->Save(project.manifestPath)) {
                Utils::Logger::Instance().Error(
                    "DaemonCommand",
                    "Failed to update XAML entries in: " + project.manifestPath.string());
            }
        }

        ProjectState CaptureState(const RegisteredProject& project) {
            ProjectState state;
            std::error_code error;

            if (std::filesystem::exists(project.manifestPath, error)) {
                state.emplace(
                    project.manifestPath,
                    FileState{
                        std::filesystem::last_write_time(project.manifestPath, error),
                        std::filesystem::file_size(project.manifestPath, error)
                    }
                );
            }

            if (std::filesystem::is_directory(project.assetRoot, error)) {
                for (std::filesystem::recursive_directory_iterator iterator(project.assetRoot, error), end;
                     iterator != end && !error;
                     iterator.increment(error)) {
                    if (!iterator->is_regular_file(error)) {
                        continue;
                    }

                    auto path = iterator->path();
                    state.emplace(
                        path,
                        FileState{
                            std::filesystem::last_write_time(path, error),
                            std::filesystem::file_size(path, error)
                        }
                    );
                }
            }

            error.clear();
            for (std::filesystem::recursive_directory_iterator iterator(
                     project.projectRoot,
                     std::filesystem::directory_options::skip_permission_denied,
                     error), end;
                 iterator != end && !error;
                 iterator.increment(error)) {
                if (iterator->is_directory(error)) {
                    continue;
                }
                if (iterator->is_regular_file(error) && iterator->path().extension() == ".xaml") {
                    auto path = iterator->path();
                    state.emplace(
                        path,
                        FileState{
                            std::filesystem::last_write_time(path, error),
                            std::filesystem::file_size(path, error)
                        }
                    );
                }
            }

            return state;
        }

        bool HasChanged(const ProjectState& previous, const ProjectState& current) {
            if (previous.size() != current.size()) {
                return true;
            }

            for (const auto& [path, state] : current) {
                auto previousFile = previous.find(path);
                if (previousFile == previous.end() || !(previousFile->second == state)) {
                    return true;
                }
            }

            return false;
        }

        void Synchronize(const RegisteredProject& project) {
            auto& logger = Utils::Logger::Instance();
            SynchronizeXamlManifest(project);
            auto command = Commands::SyncResourcesCommand(
                project.manifestPath.string(),
                project.generatedSourcePath.parent_path().string()
            );

            if (command.Execute() != 0) {
                logger.Error(
                    "DaemonCommand",
                    "Resource synchronization failed for: " + project.projectRoot.string()
                );
            }
        }
    }

    DaemonCommand::DaemonCommand(bool background)
        : m_background(background) {
    }

    bool DaemonCommand::StartDetached() {
        auto executablePath = ToolchainBootstrap::GetToolExecutablePath();
        if (!Utils::FileSystem::FileExists(executablePath)) {
            executablePath = Utils::FileSystem::GetExecutablePath().value_or(std::filesystem::path{});
        }

        if (executablePath.empty() || !Utils::FileSystem::FileExists(executablePath)) {
            Utils::Logger::Instance().Error(
                "DaemonCommand",
                "The NeoPM executable could not be resolved for daemon startup"
            );
            return false;
        }

        if (!Utils::Process::StartDetached(executablePath, "daemon --background")) {
            Utils::Logger::Instance().Error("DaemonCommand", "Failed to start the NeoPM daemon process");
            return false;
        }

        Utils::Logger::Instance().Info("DaemonCommand", "NeoPM daemon start requested");
        return true;
    }

    bool DaemonCommand::EnsureRunning() {
        return StartDetached();
    }

    int DaemonCommand::Execute() {
        auto& logger = Utils::Logger::Instance();
        if (!m_background && !StartDetached()) {
            logger.Error("DaemonCommand", "Failed to detach the NeoPM daemon from the current console");
            return 1;
        }
        if (!m_background) {
            return 0;
        }

        Utils::NamedMutex mutex("NeoWPF.NeoPM.Daemon");
        if (!mutex.Acquire()) {
            logger.Info("DaemonCommand", "NeoPM daemon is already running");
            return 0;
        }

        auto projects = ProjectRegistry::Load();
        logger.Info("DaemonCommand", "NeoPM daemon started with " + std::to_string(projects.size()) + " registered project(s)");

        std::map<std::filesystem::path, ProjectState> states;
        for (const auto& project : projects) {
            Synchronize(project);
            states.emplace(project.projectRoot, CaptureState(project));
        }

        while (true) {
            projects = ProjectRegistry::Load();
            for (const auto& project : projects) {
                auto currentState = CaptureState(project);
                auto previousState = states.find(project.projectRoot);
                if (previousState == states.end() || HasChanged(previousState->second, currentState)) {
                    Synchronize(project);
                    states[project.projectRoot] = CaptureState(project);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        return 0;
    }

    const char* DaemonCommand::GetName() const {
        return "daemon";
    }

    const char* DaemonCommand::GetDescription() const {
        return "Run the NeoPM resource daemon";
    }
}