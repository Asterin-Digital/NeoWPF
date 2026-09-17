#include <NeoPM/Core/Daemon/ProjectRegistry.h>
#include <NeoPM/Core/ToolchainBootstrap.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>

namespace NeoPM::Core::Daemon {
    namespace {
        using json = nlohmann::json;

        std::filesystem::path Normalize(const std::filesystem::path& path) {
            std::error_code error;
            auto absolute = std::filesystem::absolute(path, error);
            return (error ? path : absolute).lexically_normal();
        }

        RegisteredProject FromJson(const json& value) {
            return {
                value.value("projectRoot", ""),
                value.value("manifestPath", ""),
                value.value("assetRoot", ""),
                value.value("generatedSourcePath", ""),
                value.value("generatedHeaderPath", "")
            };
        }

        json ToJson(const RegisteredProject& project) {
            return {
                {"projectRoot", Normalize(project.projectRoot).string()},
                {"manifestPath", Normalize(project.manifestPath).string()},
                {"assetRoot", Normalize(project.assetRoot).string()},
                {"generatedSourcePath", Normalize(project.generatedSourcePath).string()},
                {"generatedHeaderPath", Normalize(project.generatedHeaderPath).string()}
            };
        }
    }

    std::filesystem::path ProjectRegistry::GetPath() {
        return ToolchainBootstrap::GetToolDirectory() / "projects.json";
    }

    std::vector<RegisteredProject> ProjectRegistry::Load() {
        std::ifstream file(GetPath());
        if (!file) {
            return {};
        }

        try {
            auto document = json::parse(file);
            std::vector<RegisteredProject> projects;
            for (const auto& value : document.value("projects", json::array())) {
                auto project = FromJson(value);
                if (!project.projectRoot.empty() && !project.manifestPath.empty()) {
                    projects.push_back(std::move(project));
                }
            }
            return projects;
        }
        catch (const json::exception& exception) {
            Utils::Logger::Instance().Error("ProjectRegistry", "Failed to read daemon registry: " + std::string(exception.what()));
            return {};
        }
    }

    bool ProjectRegistry::Register(const RegisteredProject& project) {
        auto registryPath = GetPath();
        if (!Utils::FileSystem::DirectoryExists(registryPath.parent_path()) &&
            !Utils::FileSystem::CreateDirectories(registryPath.parent_path())) {
            Utils::Logger::Instance().Error("ProjectRegistry", "Failed to create NeoWPF daemon directory");
            return false;
        }

        auto projects = Load();
        auto existing = std::find_if(projects.begin(), projects.end(), [&](const auto& value) {
            return Normalize(value.projectRoot) == Normalize(project.projectRoot);
        });
        if (existing != projects.end()) {
            *existing = project;
        }
        else {
            projects.push_back(project);
        }

        json document = {{"projects", json::array()}};
        for (const auto& value : projects) {
            document["projects"].push_back(ToJson(value));
        }

        auto temporaryPath = registryPath;
        temporaryPath += ".tmp";
        std::ofstream file(temporaryPath, std::ios::out | std::ios::trunc);
        if (!file) {
            Utils::Logger::Instance().Error("ProjectRegistry", "Failed to write daemon registry");
            return false;
        }
        file << document.dump(2);
        file.close();

        std::error_code error;
        std::filesystem::remove(registryPath, error);
        std::filesystem::rename(temporaryPath, registryPath, error);
        if (error) {
            Utils::Logger::Instance().Error("ProjectRegistry", "Failed to replace daemon registry: " + error.message());
            return false;
        }
        return true;
    }
}