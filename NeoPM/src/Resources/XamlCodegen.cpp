#include <NeoPM/Resources/XamlCodegen.h>
#include <NeoPM/Utils/Logger.h>
#include <NeoCM/Core/Compiler/CppEmitter.h>
#include <NeoCM/Core/Compiler/NamlCompiler.h>
#include <NeoCM/Core/Compiler/ResolvedXaml.h>
#include <NeoNaml/NamlArtifact.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <cctype>
#include <fstream>
#include <system_error>
#include <unordered_set>

namespace NeoPM::Core::Resources {
namespace {

std::string ComponentIdentifier(std::filesystem::path path)
{
    auto identifier = path.replace_extension().generic_string();
    for (char& character : identifier) {
        if (!std::isalnum(static_cast<unsigned char>(character))) {
            character = '_';
        }
    }
    if (identifier.empty() || std::isdigit(static_cast<unsigned char>(identifier.front()))) {
        identifier.insert(identifier.begin(), '_');
    }
    return identifier;
}

bool WriteGeneratedFile(const std::filesystem::path& path,
                        const std::string& content,
                        NeoPM::Utils::Logger& logger)
{
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        logger.Error("XamlCodegen", "Failed to create generated directory: " + error.message());
        return false;
    }

    const auto temporaryPath = path.string() + ".tmp";
    std::ofstream output(temporaryPath, std::ios::binary | std::ios::trunc);
    if (!output) {
        logger.Error("XamlCodegen", "Failed to open generated file: " + path.string());
        return false;
    }
    output << content;
    output.close();
    if (!output) {
        logger.Error("XamlCodegen", "Failed to write generated file: " + path.string());
        return false;
    }

    std::filesystem::remove(path, error);
    std::filesystem::rename(temporaryPath, path, error);
    if (error) {
        logger.Error("XamlCodegen", "Failed to finalize generated file: " + error.message());
        return false;
    }
    return true;
}

bool RemoveStaleFiles(const std::filesystem::path& root,
                      const std::unordered_set<std::filesystem::path>& expected,
                      const std::string& suffix,
                      const std::string& filenameMarker,
                      NeoPM::Utils::Logger& logger)
{
    std::error_code error;
    if (!std::filesystem::is_directory(root, error)) {
        return true;
    }

    for (std::filesystem::recursive_directory_iterator iterator(root, error), end;
         iterator != end && !error;
         iterator.increment(error)) {
        if (!iterator->is_regular_file(error) || iterator->path().extension() != suffix ||
            (!filenameMarker.empty() && iterator->path().filename().string().find(filenameMarker) == std::string::npos)) {
            continue;
        }
        if (expected.find(iterator->path().lexically_normal()) == expected.end()) {
            std::filesystem::remove(iterator->path(), error);
            if (error) {
                logger.Error("XamlCodegen", "Failed to remove stale generated file: " + iterator->path().string());
                return false;
            }
        }
    }
    return !error;
}

}

bool XamlCodegen::Generate(const ResourceManifest& manifest,
                           const std::filesystem::path& projectRoot,
                           const std::string& projectName) const
{
    auto& logger = Utils::Logger::Instance();
    const auto artifactRoot = projectRoot / "Generated" / ".naml";
    const auto headerRoot = projectRoot / "include" / projectName;
    const auto sourceRoot = projectRoot / "Generated";
    std::unordered_set<std::filesystem::path> expectedArtifacts;
    std::unordered_set<std::filesystem::path> expectedHeaders;
    std::unordered_set<std::filesystem::path> expectedSources;
    std::vector<std::filesystem::path> publicHeaders;

    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    for (const auto& entry : manifest.GetXamlEntries()) {
        const auto inputPath = projectRoot / entry.path;
        const auto artifactPath = artifactRoot / (entry.path + ".g.naml");
        const auto headerPath = headerRoot / (entry.path + ".h");
        const auto sourcePath = sourceRoot / (entry.path + ".g.cpp");
        expectedArtifacts.insert(artifactPath.lexically_normal());
        expectedHeaders.insert(headerPath.lexically_normal());
        expectedSources.insert(sourcePath.lexically_normal());
        publicHeaders.push_back(headerPath.lexically_normal());

        std::ifstream source(inputPath, std::ios::binary | std::ios::ate);
        if (!source) {
            logger.Error("XamlCodegen", "Failed to open XAML source: " + inputPath.string());
            return false;
        }
        const auto size = source.tellg();
        source.seekg(0, std::ios::beg);
        std::string content(static_cast<size_t>(size), '\0');
        if (size > 0 && !source.read(content.data(), size)) {
            logger.Error("XamlCodegen", "Failed to read XAML source: " + inputPath.string());
            return false;
        }

        const auto namlResult = NeoCM::Core::Compiler::CompileXaml(content, inputPath.string());
        if (!namlResult.success) {
            logger.Error("XamlCodegen", "XAML compilation failed for " + inputPath.string() + ": " + namlResult.error);
            return false;
        }

        NeoWPF::UI::Markup::Xaml::RegisterXamlMarkupExtensions();
        const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(content);
        const auto resolved = NeoCM::Core::Compiler::ResolveXaml(roots, inputPath.string());
        if (!resolved.success) {
            logger.Error("XamlCodegen", "XAML resolution failed for " + inputPath.string() + ": " + resolved.error);
            return false;
        }

        const auto generated = NeoCM::Core::Compiler::EmitCppComponent(
            resolved.document,
            {projectName,
             ComponentIdentifier(std::filesystem::path(entry.path)),
             (std::filesystem::path(projectName) / (entry.path + ".h")).generic_string()});
        if (!generated.success) {
            logger.Error("XamlCodegen", "XAML C++ generation failed for " + inputPath.string() + ": " + generated.error);
            return false;
        }

        if (!WriteGeneratedFile(headerPath, generated.header, logger) ||
            !WriteGeneratedFile(sourcePath, generated.source, logger)) {
            return false;
        }

        if (!WriteGeneratedFile(artifactPath, NeoNaml::SerializeNaml(namlResult.artifact), logger)) {
            return false;
        }
    }

    std::string umbrella = "#pragma once\n\n";
    for (const auto& header : publicHeaders) {
        const auto relative = std::filesystem::relative(header, headerRoot).generic_string();
        umbrella += "#include <" + projectName + "/" + relative + ">\n";
    }
    if (!WriteGeneratedFile(headerRoot / "Xaml.h", umbrella, logger)) {
        return false;
    }
    expectedHeaders.insert((headerRoot / "Xaml.h").lexically_normal());

    if (!RemoveStaleFiles(artifactRoot, expectedArtifacts, ".naml", {}, logger) ||
        !RemoveStaleFiles(headerRoot, expectedHeaders, ".h", ".xaml.h", logger) ||
        !RemoveStaleFiles(sourceRoot, expectedSources, ".cpp", ".xaml.g.cpp", logger)) {
        return false;
    }

    return true;
}

}
