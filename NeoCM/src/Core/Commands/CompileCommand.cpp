#include <NeoCM/Core/Commands/CompileCommand.h>
#include <NeoNaml/NamlArtifact.h>
#include <NeoCM/Core/Compiler/NamlCompiler.h>
#include <NeoCM/Utils/Logger.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace NeoCM::Core::Commands {

CompileCommand::CompileCommand(CompileArguments arguments)
    : m_arguments(std::move(arguments))
{
}

int CompileCommand::Execute()
{
    return CompileFile();
}

int CompileCommand::CompileFile() const
{
    const std::filesystem::path inputPath = m_arguments.inputPath;
    if (!std::filesystem::exists(inputPath)) {
        NeoCM::Utils::Logger::Instance().Error("Input file does not exist: " + m_arguments.inputPath);
        return 1;
    }

    std::ifstream input(inputPath, std::ios::binary | std::ios::ate);
    if (!input) {
        NeoCM::Utils::Logger::Instance().Error("Failed to open input file: " + m_arguments.inputPath);
        return 1;
    }

    const std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    std::string content(static_cast<size_t>(size), '\0');
    if (!input.read(content.data(), size)) {
        NeoCM::Utils::Logger::Instance().Error("Failed to read input file: " + m_arguments.inputPath);
        return 1;
    }

    const std::filesystem::path outputPath = m_arguments.outputPath.value_or(DefaultOutputPath(m_arguments.inputPath));
    if (!outputPath.parent_path().empty()) {
        std::filesystem::create_directories(outputPath.parent_path());
    }

    const auto compileResult = NeoCM::Core::Compiler::CompileXaml(content, inputPath.string());
    if (!compileResult.success) {
        NeoCM::Utils::Logger::Instance().Error(compileResult.error);
        return 1;
    }

    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output) {
        NeoCM::Utils::Logger::Instance().Error("Failed to open output file: " + outputPath.string());
        return 1;
    }

    output << NeoNaml::SerializeNaml(compileResult.artifact);

    NeoCM::Utils::Logger::Instance().Info("Compiled XAML to: " + outputPath.string());
    return 0;
}

std::string CompileCommand::DefaultOutputPath(const std::string& inputPath)
{
    const std::filesystem::path sourcePath = inputPath;
    std::filesystem::path outputPath = sourcePath;
    outputPath += ".g.naml";
    return outputPath.string();
}

}
