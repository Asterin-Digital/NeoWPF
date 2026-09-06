#include <NeoCM/Core/Compiler/CppEmitter.h>
#include <NeoCM/Core/Compiler/NamlCompiler.h>
#include <NeoCM/Core/Compiler/ResolvedXaml.h>

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    std::ostringstream content;
    content << input.rdbuf();
    return content.str();
}

bool WriteFile(const std::filesystem::path& path, const std::string& content)
{
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return false;
    }

    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }
    output << content;
    return static_cast<bool>(output);
}

}

int main(int argc, char* argv[])
{
    if (argc != 3 && argc != 5) {
        std::cerr << "usage: NeoWPF.XamlGenerator <input.xaml> <output-directory> [namespace component]\n";
        return 1;
    }

    const std::filesystem::path inputPath = argv[1];
    const std::filesystem::path outputDirectory = argv[2];
    const std::string namespaceName = argc == 5 ? argv[3] : "NeoWPF::Generated";
    const std::string componentName = argc == 5 ? argv[4] : inputPath.stem().string();
    const std::string generatedName = inputPath.stem().string() + ".xaml.g";

    if (!std::filesystem::is_regular_file(inputPath)) {
        std::cerr << "input XAML does not exist: " << inputPath.string() << '\n';
        return 1;
    }

    const auto source = ReadFile(inputPath);
    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    const auto naml = NeoCM::Core::Compiler::CompileXaml(source, inputPath.string());
    if (!naml.success) {
        std::cerr << naml.error << '\n';
        return 1;
    }

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(source);
    const auto resolved = NeoCM::Core::Compiler::ResolveXaml(roots, inputPath.string());
    if (!resolved.success) {
        std::cerr << resolved.error << '\n';
        return 1;
    }

    const auto generated = NeoCM::Core::Compiler::EmitCppComponent(
        resolved.document,
        {namespaceName, componentName, generatedName + ".h"});
    if (!generated.success) {
        std::cerr << generated.error << '\n';
        return 1;
    }

    const auto headerPath = outputDirectory / (generatedName + ".h");
    const auto sourcePath = outputDirectory / (generatedName + ".cpp");
    const auto artifactPath = outputDirectory / (generatedName + ".naml");
    if (!WriteFile(headerPath, generated.header) ||
        !WriteFile(sourcePath, generated.source) ||
        !WriteFile(artifactPath, NeoNaml::SerializeNaml(naml.artifact))) {
        std::cerr << "failed to write generated XAML outputs\n";
        return 1;
    }

    return 0;
}
