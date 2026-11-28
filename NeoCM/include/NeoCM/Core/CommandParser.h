/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoCM/Core/ICommand.h>
#include <NeoWPF/Feedback/Result.h>
#include <memory>
#include <string>
#include <vector>

namespace NeoCM::Core {

struct CommandArguments {
    std::string commandName;
    std::string inputPath;
    std::string outputPath;
    bool showHelp = false;
    bool showVersion = false;
};

class CommandParser {
public:
    static CommandParser& Instance();

    std::unique_ptr<ICommand> Parse(int argc, wchar_t* argv[]);
    const char* GetVersion() const;
    void PrintUsage() const;

private:
    CommandParser() = default;
    ~CommandParser() = default;

    CommandParser(const CommandParser&) = delete;
    CommandParser& operator=(const CommandParser&) = delete;

    /*
        @summary
        Parses command-line arguments while retaining conversion failures.

        @param argc
        Number of command-line arguments.

        @param argv
        Command-line arguments.

        @returns
        Parsed arguments, or the reason parsing failed.

        @internal
    */
    NeoWPF::Result<CommandArguments> ParseArguments(int argc, wchar_t* argv[]) const;

    /*
        @summary
        Converts a wide command-line value to UTF-8.

        @param wide
        Value to convert.

        @returns
        UTF-8 value, or the conversion error.

        @internal
    */
    NeoWPF::Result<std::string> WideStringToString(const std::wstring& wide) const;
    void PrintCommandHelp(const std::string& commandName) const;
};

}
