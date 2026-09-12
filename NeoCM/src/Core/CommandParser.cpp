#include <NeoCM/Core/CommandParser.h>
#include <NeoCM/Core/Commands/CompileCommand.h>
#include <NeoCM/Utils/Logger.h>

#include <algorithm>
#include <cwchar>
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
    const auto args = ParseArguments(argc, argv);

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

CommandArguments CommandParser::ParseArguments(int argc, wchar_t* argv[]) const
{
    CommandArguments args;
    std::vector<std::wstring> values;
    for (int i = 1; i < argc; ++i) {
        values.emplace_back(argv[i]);
    }

    if (values.empty()) {
        return args;
    }

    args.commandName = WideStringToString(values[0].c_str());

    for (size_t i = 1; i < values.size(); ++i) {
        const std::string value = WideStringToString(values[i].c_str());
        if (value == "--help" || value == "-h") {
            args.showHelp = true;
        } else if (value == "--version" || value == "-v") {
            args.showVersion = true;
        } else if (value == "--output" || value == "-o") {
            if (i + 1 < values.size()) {
                args.outputPath = WideStringToString(values[++i].c_str());
            }
        } else if (args.inputPath.empty()) {
            args.inputPath = value;
        }
    }

    return args;
}

std::string CommandParser::WideStringToString(const wchar_t* wide) const
{
    if (!wide) {
        return {};
    }

    const int length = static_cast<int>(std::wcslen(wide));
    std::string out;
    out.reserve(static_cast<size_t>(length));
    for (int i = 0; i < length; ++i) {
        out.push_back(static_cast<char>(wide[i]));
    }
    return out;
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
