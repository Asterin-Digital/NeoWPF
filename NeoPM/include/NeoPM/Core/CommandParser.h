#pragma once

#include "ICommand.h"
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace NeoPM::Core {
    struct CommandArguments {
        std::string commandName;
        std::string projectName;           // For 'new' command
        std::string manifestPath;          // For 'sync-resources' command
        std::string projectPath;           // For 'repair' command
        std::optional<std::string> outputPath;
        std::optional<std::string> copyOutputPath;
        std::optional<std::string> templateName;
        std::string projectType = "vcxproj";
        std::string sdkSource = "production";
        std::string sdkConfiguration = "debug";
        bool sdkConfigurationSpecified = false;
        std::optional<std::string> sdkIncludePath;
        std::optional<std::string> sdkLibraryPath;
        bool daemonBackground = false;

        bool showHelp = false;
        bool showVersion = false;
    };

    class CommandParser {
    public:
        static CommandParser& Instance();

        std::unique_ptr<ICommand> Parse(int argc, char* argv[]);
        const char* GetVersion() const;
        void PrintUsage() const;

    private:
        CommandParser() = default;
        ~CommandParser() = default;

        CommandParser(const CommandParser&) = delete;
        CommandParser& operator=(const CommandParser&) = delete;

        CommandArguments ParseArguments(int argc, char* argv[]);
        void PrintCommandHelp(const std::string& commandName) const;
    };
}
