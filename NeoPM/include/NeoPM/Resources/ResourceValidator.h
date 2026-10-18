/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include "ResourceManifest.h"
#include <filesystem>
#include <string>
#include <vector>

namespace NeoPM::Core::Resources {
    struct ValidationError {
        std::string message;
        int entryIndex = -1;  // -1 if not related to a specific entry
        bool isWarning = false;
    };

    class ResourceValidator {
    public:
        // Validate a manifest against a project root
        // Returns true if no fatal errors, false if validation failed
        bool Validate(
            const ResourceManifest& manifest,
            const std::filesystem::path& projectRoot,
            std::vector<ValidationError>& outErrors
        );

        // Validate a single entry
        bool ValidateEntry(
            const ResourceEntry& entry,
            const std::filesystem::path& projectRoot,
            std::vector<ValidationError>& outErrors
        );

    private:
        // Helper validations
        bool ValidateKey(const std::string& key, std::vector<ValidationError>& outErrors) const;
        bool ValidateFilePath(
            const std::string& path,
            const std::filesystem::path& projectRoot,
            std::vector<ValidationError>& outErrors
        ) const;
        bool ValidateType(ResourceType type) const;
    };
}
