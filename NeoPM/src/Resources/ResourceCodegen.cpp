#include <NeoPM/Resources/ResourceCodegen.h>
#include <NeoPM/Resources/XamlCodegen.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <system_error>

namespace NeoPM::Core::Resources {
    bool ResourceCodegen::Generate(
        const ResourceManifest& manifest,
        const std::filesystem::path& projectRoot,
        const std::string& projectName,
        const std::filesystem::path& outputDir,
        const std::optional<std::filesystem::path>& copyOutputDir
    ) {
        auto& logger = Utils::Logger::Instance();

        if (!XamlCodegen{}.Generate(manifest, projectRoot, projectName)) {
            return false;
        }


        std::error_code ec;

        std::filesystem::create_directories(outputDir, ec);
        if (ec || !std::filesystem::is_directory(outputDir)) {
            logger.Error("ResourceCodegen", "Failed to create output directory: " + outputDir.string() + " (" + ec.message() + ")");
            return false;
        }

        auto headerDir = projectRoot / "include" / projectName / "Generated";
        std::filesystem::create_directories(headerDir, ec);
        if (!std::filesystem::exists(headerDir)) {
            logger.Error("ResourceCodegen", "Failed to create header directory: " + headerDir.string());
            return false;
        }

        auto headerContent = GenerateHeader(manifest, projectName);
        auto headerPath = headerDir / "Resources.generated.h";

        std::filesystem::create_directories(headerPath.parent_path(), ec);
        std::ofstream headerFile(headerPath, std::ios::out | std::ios::trunc);
        if (!headerFile.is_open()) {
            logger.Error("ResourceCodegen", "Failed to open header file for writing: " + headerPath.string());
            return false;
        }
        headerFile << headerContent;
        headerFile.close();

        logger.Info("ResourceCodegen", "Generated header: " + headerPath.string());


        auto sourceContent = GenerateSource(manifest, projectName, projectRoot);
        auto sourcePath = outputDir / "Resources.generated.cpp";

        std::filesystem::create_directories(sourcePath.parent_path(), ec);
        std::ofstream sourceFile(sourcePath, std::ios::out | std::ios::trunc);
        if (!sourceFile.is_open()) {
            logger.Error("ResourceCodegen", "Failed to open source file for writing: " + sourcePath.string());
            return false;
        }
        sourceFile << sourceContent;
        sourceFile.close();

        logger.Info("ResourceCodegen", "Generated source: " + sourcePath.string());

        if (copyOutputDir.has_value()) {
            const auto externalResources = manifest.GetExternalResources();
            const auto runtimeOutputRoot = std::filesystem::absolute(copyOutputDir->empty() ? projectRoot : *copyOutputDir);
            const auto runtimeResourcesDir = runtimeOutputRoot / "Resources";
            for (const auto& entry : externalResources) {
                const auto sourceFile = projectRoot / "Resources" / "Assets" / entry.path;
                const auto destFile = runtimeResourcesDir / entry.path;

                std::error_code copyError;
                std::filesystem::create_directories(destFile.parent_path(), copyError);
                if (copyError) {
                    logger.Error("ResourceCodegen", "Failed to create destination directory for external resource: " + copyError.message() + " (path: " + destFile.parent_path().string() + ")");
                    return false;
                }

                if (!std::filesystem::exists(sourceFile)) {
                    logger.Error("ResourceCodegen", "Missing external resource file: " + sourceFile.string());
                    return false;
                }

                std::filesystem::copy_file(sourceFile, destFile, std::filesystem::copy_options::overwrite_existing, copyError);
                if (copyError) {
                    logger.Error("ResourceCodegen", "Failed to copy external resource file: " + copyError.message() + " (source: " + sourceFile.string() + ", destination: " + destFile.string() + ")");
                    return false;
                }
            }
        }

        return true;
    }

    std::string ResourceCodegen::GenerateHeader(
        const ResourceManifest& manifest,
        const std::string& projectName
    ) const {
        auto oss = std::ostringstream{};

        oss << "#pragma once\n\n";
        oss << "#include <NeoWPF/NeoWPF.h>\n\n";
        oss << "namespace " << projectName << "::Resources {\n";
        oss << "    // Resource keys for this project\n\n";

        const auto& entries = manifest.GetEntries();
        for (const auto& entry : entries) {
            const auto keyName = entry.path.empty() ? entry.key : entry.path;
            oss << "    inline constexpr NeoWPF::ResourceKey " << entry.GetIdentifier()
                << " = NEOWPF_RESOURCE_KEY(\"" << keyName << "\");\n";
        }
            for (const auto& entry : manifest.GetXamlEntries()) {
                const auto xamlResource = ResourceEntry{entry.key, ResourceType::Xaml, entry.path};
                oss << "    inline constexpr NeoWPF::ResourceKey " << xamlResource.GetIdentifier()
                << " = NEOWPF_RESOURCE_KEY(\"" << entry.key << "\");\n";
            }

        oss << "\n";
        oss << "    // Register all resources with the ResourceManager\n";
        oss << "    // Called once at application startup from App.xaml.cpp\n";
        oss << "    void RegisterAll();\n";
        oss << "}\n";

        return oss.str();
    }

    std::string ResourceCodegen::GenerateSource(
        const ResourceManifest& manifest,
        const std::string& projectName,
        const std::filesystem::path& projectRoot
    ) const {
        auto oss = std::ostringstream{};

        oss << "#include <" << projectName << "/Generated/Resources.generated.h>\n";
        oss << "#include <NeoWPF/NeoWPF.h>\n\n";

        oss << "namespace " << projectName << "::Resources {\n";
        oss << "    namespace {\n";

        const auto& entries = manifest.GetEntries();


        for (const auto& entry : entries) {
            if (entry.embed) {
                auto assetPath = projectRoot / "Resources" / "Assets" / entry.path;
                auto hexData = BinaryToHexString(assetPath);

                oss << "        // " << entry.key << " (" << entry.path << ")\n";
                oss << "        const unsigned char " << GetDataArrayName(entry.GetIdentifier())
                    << "[] = {\n";
                oss << "            " << hexData << "\n";
                oss << "        };\n";
                oss << "        constexpr size_t " << GetDataArrayName(entry.GetIdentifier())
                    << "_size = sizeof(" << GetDataArrayName(entry.GetIdentifier()) << ");\n\n";
            }
        }

        for (const auto& entry : manifest.GetXamlEntries()) {
            const auto xamlResource = ResourceEntry{entry.key, ResourceType::Xaml, entry.path};
            const auto artifactPath = projectRoot / "Generated" / ".naml" / (entry.path + ".g.naml");
            oss << "        // " << entry.key << " (compiled XAML)\n";
            oss << "        const unsigned char " << GetDataArrayName(xamlResource.GetIdentifier()) << "[] = {\n";
            oss << "            " << BinaryToHexString(artifactPath) << "\n";
            oss << "        };\n";
            oss << "        constexpr size_t " << GetDataArrayName(xamlResource.GetIdentifier())
                << "_size = sizeof(" << GetDataArrayName(xamlResource.GetIdentifier()) << ");\n\n";
        }

        oss << "    }\n\n";

        oss << "    void RegisterAll() {\n";
        oss << "        auto& manager = NeoWPF::ResourceManager::Instance();\n";
        oss << "        auto& provider = NeoWPF::EmbeddedResourceProvider::Instance();\n\n";
        oss << "        static bool registered = false;\n";
        oss << "        if (!registered) {\n";
        oss << "            manager.RegisterProviderReference(provider);\n";
        oss << "            registered = true;\n";
        oss << "        }\n\n";

        for (const auto& entry : entries) {
            if (entry.embed) {
                oss << "        provider.Register(\n";
                oss << "            " << entry.GetIdentifier() << ",\n";
                oss << "            NeoWPF::ResourceType::" << ResourceEntry::TypeToString(entry.type) << ",\n";
                oss << "            " << GetDataArrayName(entry.GetIdentifier()) << ",\n";
                oss << "            " << GetDataArrayName(entry.GetIdentifier()) << "_size\n";
                oss << "        );\n";
            }
        }

        for (const auto& entry : manifest.GetXamlEntries()) {
            const auto xamlResource = ResourceEntry{entry.key, ResourceType::Xaml, entry.path};
            oss << "        provider.Register(\n";
            oss << "            " << xamlResource.GetIdentifier() << ",\n";
            oss << "            NeoWPF::ResourceType::Xaml,\n";
            oss << "            " << GetDataArrayName(xamlResource.GetIdentifier()) << ",\n";
            oss << "            " << GetDataArrayName(xamlResource.GetIdentifier()) << "_size\n";
            oss << "        );\n";
        }

        oss << "    }\n";
        oss << "}\n";

        return oss.str();
    }

    std::string ResourceCodegen::BinaryToHexString(const std::filesystem::path& filePath) const {
        auto file = std::ifstream(filePath, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }

        auto oss = std::ostringstream{};
        unsigned char byte;
        auto count = 0;
        auto bytesPerLine = 16;

        while (file.read(reinterpret_cast<char*>(&byte), 1)) {
            if (count > 0) {
                oss << ",";
                if (count % bytesPerLine == 0) {
                    oss << "\n            ";
                } else {
                    oss << " ";
                }
            }
            oss << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
            count++;
        }

        file.close();
        return oss.str();
    }

    std::string ResourceCodegen::GenerateResourceRegistration(
        const ResourceEntry& entry,
        const std::string& varName
    ) const {
        auto oss = std::ostringstream{};
        oss << "    provider.Register(\n";
        oss << "        " << varName << ",\n";
        oss << "        NeoWPF::ResourceType::" << ResourceEntry::TypeToString(entry.type) << ",\n";
        oss << "        " << GetDataArrayName(varName) << ",\n";
        oss << "        " << GetDataArrayName(varName) << "_size\n";
        oss << "    );\n";
        return oss.str();
    }
}
