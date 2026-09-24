/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoCM/Core/ICommand.h>
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

    CommandArguments ParseArguments(int argc, wchar_t* argv[]) const;
    std::string WideStringToString(const wchar_t* wide) const;
    void PrintCommandHelp(const std::string& commandName) const;
};

}
