#include <NeoCM/Core/CommandParser.h>
#include <NeoCM/Core/Commands/CompileCommand.h>
#include <NeoCM/Utils/Logger.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace NeoCM::Core {

CommandParser& CommandParser::Instance()
{
    static CommandParser parser;
    return parser;
}

std::unique_ptr<ICommand> CommandParser::Parse(int argc, wchar_t* argv[])
{
    auto parsedArguments = ParseArguments(argc, argv);
    if (parsedArguments.Fail()) {
        NeoCM::Utils::Logger::Instance().Error(
            "Failed to parse command-line arguments: " + parsedArguments.Error());
        return nullptr;
    }
    const auto& args = parsedArguments.Value();

    if (args.showHelp || args.commandName.empty()) {
        PrintUsage();
        return nullptr;
    }

    if (args.showVersion) {
        std::cout << GetVersion() << '\n';
        return nullptr;
    }

    if (args.commandName == "compile") {
        NeoCM::Core::Commands::CompileArguments compileArguments;
        compileArguments.inputPath = args.inputPath;
        if (!args.outputPath.empty()) {
            compileArguments.outputPath = args.outputPath;
        }
        compileArguments.showHelp = args.showHelp;
        return std::make_unique<NeoCM::Core::Commands::CompileCommand>(std::move(compileArguments));
    }

    NeoCM::Utils::Logger::Instance().Error("Unknown command: " + args.commandName);
    PrintUsage();
    return nullptr;
}

const char* CommandParser::GetVersion() const
{
    return "NeoCM 0.1.0";
}

void CommandParser::PrintUsage() const
{
    NeoCM::Utils::Logger::Instance().PrintUsage();
}

NeoWPF::Result<CommandArguments> CommandParser::ParseArguments(int argc, wchar_t* argv[]) const
{
    if (argc < 0 || (argc > 0 && !argv)) {
        return NeoWPF::Fail<std::string>{"Invalid command-line argument array."};
    }

    CommandArguments args;
    std::vector<std::wstring> values;
    for (int i = 1; i < argc; ++i) {
        if (!argv[i]) {
            return NeoWPF::Fail<std::string>{
                "Command-line argument at index " + std::to_string(i) + " is null."};
        }
        values.emplace_back(argv[i]);
    }

    if (values.empty()) {
        return NeoWPF::Success<CommandArguments>{std::move(args)};
    }

    auto commandName = WideStringToString(values[0]);
    if (commandName.Fail()) {
        return NeoWPF::Fail<std::string>{
            "Failed to convert command name: " + commandName.Error()};
    }
    args.commandName = std::move(commandName).Value();

    for (size_t i = 1; i < values.size(); ++i) {
        auto valueResult = WideStringToString(values[i]);
        if (valueResult.Fail()) {
            return NeoWPF::Fail<std::string>{
                "Failed to convert command-line argument at index " +
                std::to_string(i + 1) + ": " + valueResult.Error()};
        }
        const std::string value = std::move(valueResult).Value();
        if (value == "--help" || value == "-h") {
            args.showHelp = true;
        } else if (value == "--version" || value == "-v") {
            args.showVersion = true;
        } else if (value == "--output" || value == "-o") {
            if (i + 1 < values.size()) {
                ++i;
                auto outputPath = WideStringToString(values[i]);
                if (outputPath.Fail()) {
                    return NeoWPF::Fail<std::string>{
                        "Failed to convert output path: " + outputPath.Error()};
                }
                args.outputPath = std::move(outputPath).Value();
            }
        } else if (args.inputPath.empty()) {
            args.inputPath = value;
        }
    }

    return NeoWPF::Success<CommandArguments>{std::move(args)};
}

NeoWPF::Result<std::string> CommandParser::WideStringToString(const std::wstring& wide) const
{
    return NeoWPF::Platform::WideToUtf8(wide);
}

void CommandParser::PrintCommandHelp(const std::string& commandName) const
{
    if (commandName == "compile") {
        NeoCM::Utils::Logger::Instance().Info("compile <input.xaml> [--output <output.naml>]\n");
        return;
    }

    NeoCM::Utils::Logger::Instance().Info("Available commands: compile\n");
}

}
