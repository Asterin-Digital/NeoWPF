#include <NeoCM/Core/Compiler/CppEmitter.h>
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
        std::cerr << "usage: GenerateComponent <input.xaml> <output-directory> [namespace component]\n";
        return 1;
    }

    const std::filesystem::path inputPath = argv[1];
    const std::filesystem::path outputDirectory = argv[2];
    const std::string namespaceName = argc == 5 ? argv[3] : "NeoCM::Tests";
    const std::string componentName = argc == 5 ? argv[4] : "MainWindow";
    const std::string generatedName = inputPath.stem().string() + ".xaml.g";
    if (!std::filesystem::exists(inputPath)) {
        std::cerr << "input XAML does not exist: " << inputPath.string() << '\n';
        return 1;
    }

    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(ReadFile(inputPath));
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

    std::error_code error;
    std::filesystem::create_directories(outputDirectory, error);
    if (error) {
        std::cerr << "failed to create output directory: " << error.message() << '\n';
        return 1;
    }

    const auto headerPath = outputDirectory / (generatedName + ".h");
    const auto sourcePath = outputDirectory / (generatedName + ".cpp");
    if (!WriteFile(headerPath, generated.header) || !WriteFile(sourcePath, generated.source)) {
        std::cerr << "failed to write generated component files\n";
        return 1;
    }

    return 0;
}
