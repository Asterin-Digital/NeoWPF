#include <NeoPM/Resources/ResourceManifest.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>
#include <nlohmann/json.hpp>
#include <fstream>

namespace NeoPM::Core::Resources {
    using json = nlohmann::json;

    std::optional<ResourceManifest> ResourceManifest::Load(const std::filesystem::path& manifestPath) {
        auto& logger = Utils::Logger::Instance();

        if (!Utils::FileSystem::FileExists(manifestPath)) {
            logger.Error("ResourceManifest", "Manifest file not found: " + manifestPath.string());
            return std::nullopt;
        }


        auto file = std::ifstream(manifestPath);
        if (!file.is_open()) {
            logger.Error("ResourceManifest", "Failed to open manifest file: " + manifestPath.string());
            return std::nullopt;
        }

        try {
            auto root = json{};
            file >> root;
            file.close();

            ResourceManifest manifest;


            if (!root.contains("resources") || !root["resources"].is_array()) {
                logger.Error("ResourceManifest", "No 'resources' array found in manifest");
                return std::nullopt;
            }

            for (const auto& entry : root.value("xaml", json::array())) {
                if (!entry.is_object() || !entry.contains("key") || !entry["key"].is_string() ||
                    !entry.contains("path") || !entry["path"].is_string()) {
                    logger.Warning("ResourceManifest", "Skipping invalid Xaml resource entry");
                    continue;
                }
                manifest.m_xamlEntries.push_back({
                    entry["key"].get<std::string>(),
                    entry["path"].get<std::string>()
                });
            }
            for (const auto& entry : root["resources"]) {
                if (!entry.is_object()) {
                    logger.Warning("ResourceManifest", "Skipping non-object entry in resources array");
                    continue;
                }

                ResourceEntry resource;


                if (!entry.contains("key") || !entry["key"].is_string()) {
                    logger.Warning("ResourceManifest", "Resource entry missing 'key' field");
                    continue;
                }
                resource.key = entry["key"].get<std::string>();


                if (!entry.contains("type") || !entry["type"].is_string()) {
                    logger.Warning("ResourceManifest", "Resource entry missing 'type' field");
                    continue;
                }
                try {
                    resource.type = ResourceEntry::StringToType(entry["type"].get<std::string>());
                } catch (const std::exception& e) {
                    logger.Warning("ResourceManifest", std::string(e.what()));
                    continue;
                }


                if (!entry.contains("path") || !entry["path"].is_string()) {
                    logger.Warning("ResourceManifest", "Resource entry missing 'path' field");
                    continue;
                }
                resource.path = entry["path"].get<std::string>();


                if (entry.contains("embed") && entry["embed"].is_boolean()) {
                    resource.embed = entry["embed"].get<bool>();
                } else {
                    resource.embed = true;
                }

                manifest.AddEntry(resource);
            }

            return manifest;

        } catch (const json::exception& e) {
            logger.Error("ResourceManifest", std::string("JSON parse error: ") + e.what());
            return std::nullopt;
        } catch (const std::exception& e) {
            logger.Error("ResourceManifest", std::string("Error loading manifest: ") + e.what());
            return std::nullopt;
        }
    }

    bool ResourceManifest::Save(const std::filesystem::path& manifestPath) const {
        using json = nlohmann::json;
        json root = { {"resources", json::array()}, {"xaml", json::array()} };
        for (const auto& entry : m_entries) {
            root["resources"].push_back({
                {"key", entry.key},
                {"type", ResourceEntry::TypeToString(entry.type)},
                {"path", entry.path},
                {"embed", entry.embed}
            });
        }
        for (const auto& entry : m_xamlEntries) {
            root["xaml"].push_back({{"key", entry.key}, {"path", entry.path}});
        }

        auto temporaryPath = manifestPath;
        temporaryPath += ".tmp";
        std::ofstream file(temporaryPath, std::ios::out | std::ios::trunc);
        if (!file) {
            return false;
        }
        file << root.dump(2) << '\n';
        file.close();

        std::error_code error;
        std::filesystem::remove(manifestPath, error);
        std::filesystem::rename(temporaryPath, manifestPath, error);
        return !error;
    }

    std::vector<ResourceEntry> ResourceManifest::GetEntriesByType(ResourceType type) const {
        std::vector<ResourceEntry> result;
        for (const auto& entry : m_entries) {
            if (entry.type == type) {
                result.push_back(entry);
            }
        }
        return result;
    }

    bool ResourceManifest::HasEmbeddedResources() const {
        for (const auto& entry : m_entries) {
            if (entry.embed) {
                return true;
            }
        }
        return false;
    }

    std::vector<ResourceEntry> ResourceManifest::GetExternalResources() const {
        std::vector<ResourceEntry> result;
        for (const auto& entry : m_entries) {
            if (!entry.embed) {
                result.push_back(entry);
            }
        }
        return result;
    }
}
