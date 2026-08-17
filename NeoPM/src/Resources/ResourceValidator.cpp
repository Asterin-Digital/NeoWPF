#include <NeoPM/Resources/ResourceValidator.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>

namespace NeoPM::Core::Resources {
    bool ResourceValidator::Validate(
        const ResourceManifest& manifest,
        const std::filesystem::path& projectRoot,
        std::vector<ValidationError>& outErrors
    ) {
        auto& logger = Utils::Logger::Instance();
        bool hasErrors = false;

        const auto& entries = manifest.GetEntries();
        if (entries.empty() && manifest.GetXamlEntries().empty()) {
            logger.Warning("ResourceValidator", "No resources defined in manifest; generating empty resource registration");
            return true;
        }

        auto index = 0;
        for (const auto& entry : entries) {
            if (!ValidateEntry(entry, projectRoot, outErrors)) {
                hasErrors = true;
            }
            index++;
        }

        for (const auto& entry : manifest.GetXamlEntries()) {
            const auto sourcePath = projectRoot / entry.path;
            if (entry.key.empty() || entry.path.empty() ||
                !std::filesystem::is_regular_file(sourcePath)) {
                outErrors.push_back({"XAML source file not found: " + sourcePath.string(), -1, false});
                hasErrors = true;
            }
        }

        return !hasErrors;
    }

    bool ResourceValidator::ValidateEntry(
        const ResourceEntry& entry,
        const std::filesystem::path& projectRoot,
        std::vector<ValidationError>& outErrors
    ) {
        auto& logger = Utils::Logger::Instance();
        bool valid = true;

        // Validate key
        if (!ValidateKey(entry.key, outErrors)) {
            valid = false;
        }

        // Validate path
        if (!ValidateFilePath(entry.path, projectRoot, outErrors)) {
            valid = false;
        }

        // Validate type
        if (!ValidateType(entry.type)) {
            outErrors.push_back({
                "Invalid resource type for key '" + entry.key + "'",
                -1,
                false
            });
            valid = false;
        }

            if (entry.type == ResourceType::Xaml && !entry.embed) {
                outErrors.push_back({
                    "Xaml resource '" + entry.key + "' must be embedded",
                    -1,
                    false
                });
                valid = false;
            }

        return valid;
    }

    bool ResourceValidator::ValidateKey(const std::string& key, std::vector<ValidationError>& outErrors) const {
        if (key.empty()) {
            outErrors.push_back({"Resource key cannot be empty", -1, false});
            return false;
        }

        // Check for valid hierarchical key format (e.g., "Icons/Play", "Fonts/Inter")
        // Keys can contain letters, numbers, slashes, underscores, hyphens
        for (char c : key) {
            if (!std::isalnum(c) && c != '/' && c != '_' && c != '-' && c != '.') {
                outErrors.push_back({
                    "Invalid character in key '" + key + "': " + c,
                    -1,
                    false
                });
                return false;
            }
        }

        return true;
    }

    bool ResourceValidator::ValidateFilePath(
        const std::string& path,
        const std::filesystem::path& projectRoot,
        std::vector<ValidationError>& outErrors
    ) const {
        if (path.empty()) {
            outErrors.push_back({"Resource path cannot be empty", -1, false});
            return false;
        }

        auto assetPath = projectRoot / "Resources" / "Assets" / path;

        if (!Utils::FileSystem::FileExists(assetPath)) {
            outErrors.push_back({
                "Asset file not found: " + assetPath.string(),
                -1,
                false
            });
            return false;
        }

        return true;
    }

    bool ResourceValidator::ValidateType(ResourceType type) const {
        return type == ResourceType::Image || type == ResourceType::Font ||
            type == ResourceType::Xaml || type == ResourceType::Raw;
    }
}
