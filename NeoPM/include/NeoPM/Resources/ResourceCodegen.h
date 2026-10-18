/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include "ResourceManifest.h"
#include <filesystem>
#include <optional>
#include <string>

namespace NeoPM::Core::Resources {
    class ResourceCodegen {
    public:
        // Generate resource header and source files
        // Returns true on success
        bool Generate(
            const ResourceManifest& manifest,
            const std::filesystem::path& projectRoot,
            const std::string& projectName,
            const std::filesystem::path& outputDir,
            const std::optional<std::filesystem::path>& copyOutputDir = std::nullopt
        );

    private:
        // Generate header file content
        std::string GenerateHeader(
            const ResourceManifest& manifest,
            const std::string& projectName
        ) const;

        // Generate source file content
        std::string GenerateSource(
            const ResourceManifest& manifest,
            const std::string& projectName,
            const std::filesystem::path& projectRoot
        ) const;

        // Read binary file and return hex string representation
        std::string BinaryToHexString(
            const std::filesystem::path& filePath
        ) const;

        // Generate function to register a single resource
        std::string GenerateResourceRegistration(
            const ResourceEntry& entry,
            const std::string& varName
        ) const;

        // Convert identifier to variable name for data array
        std::string GetDataArrayName(const std::string& identifier) const {
            return identifier + "_Data";
        }
    };
}
