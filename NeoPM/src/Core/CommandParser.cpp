#include <NeoPM/Core/CommandParser.h>
#include <NeoPM/Core/Daemon/DaemonCommand.h>
#include <NeoPM/Core/Commands/NewCommand.h>
#include <NeoPM/Core/Commands/SyncResourcesCommand.h>
#include <NeoPM/Core/Commands/RefreshSdkCommand.h>
#include <NeoPM/Core/Commands/RepairCommand.h>
#include <NeoPM/Core/ToolchainBootstrap.h>
#include <NeoPM/Utils/Logger.h>
#include <NeoPM/Utils/TextEncoding.h>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace NeoPM::Core {
    CommandParser& CommandParser::Instance() {
        static CommandParser instance;
        return instance;
    }

    std::unique_ptr<ICommand> CommandParser::Parse(int argc, char* argv[]) {
        auto& logger = Utils::Logger::Instance();

        if (argc < 2) {
            PrintUsage();
            return nullptr;
        }

        CommandArguments args = ParseArguments(argc, argv);

        if (args.commandName == "daemon --background") {
            args.commandName = "daemon";
            args.daemonBackground = true;
        }

        if (args.showHelp) {
            if (!args.commandName.empty()) {
                PrintCommandHelp(args.commandName);
            }
            else {
                PrintUsage();
            }
            return nullptr;
        }

        if (args.showVersion) {
            std::cout << "NeoPM version " << GetVersion() << std::endl;
            return nullptr;
        }

        if (!args.commandName.empty() && 
            args.commandName != "install-tools" && 
            args.commandName != "bootstrap" && 
            args.commandName != "refresh-sdk" &&
            args.commandName != "repair") {
            if (!NeoPM::Core::ToolchainBootstrap::EnsureInstalled()) {
                logger.Error("CommandParser", "Failed to install NeoWPF LocalAppData toolchain bootstrap");
                return nullptr;
            }
        }

        if (args.commandName == "new") {
            if (args.projectName.empty()) {
                logger.Error("CommandParser", "Project name is required for 'new' command");
                PrintCommandHelp("new");
                return nullptr;
            }
            if (args.sdkSource == "user" &&
                (!args.sdkConfigurationSpecified || !args.sdkIncludePath || !args.sdkLibraryPath)) {
                logger.Error(
                    "CommandParser",
                    "User SDKs require --sdk-config, --sdk-include, and --sdk-lib"
                );
                PrintCommandHelp("new");
                return nullptr;
            }
            return std::make_unique<Commands::NewCommand>(
                args.projectName,
                args.outputPath,
                args.projectType,
                args.sdkSource,
                args.sdkConfiguration,
                args.sdkIncludePath,
                args.sdkLibraryPath
            );
        }

        if (args.commandName == "daemon") {
            return std::make_unique<Daemon::DaemonCommand>(args.daemonBackground);
        }

        if (args.commandName == "sync-resources") {
            if (args.manifestPath.empty()) {
                logger.Error("CommandParser", "Manifest path is required for 'sync-resources' command");
                PrintCommandHelp("sync-resources");
                return nullptr;
            }
            return std::make_unique<Commands::SyncResourcesCommand>(args.manifestPath, args.outputPath, std::nullopt, args.copyOutputPath);
        }

        if (args.commandName == "refresh-sdk") {
            return std::make_unique<Commands::RefreshSdkCommand>();
        }

        if (args.commandName == "repair") {
            if (args.projectPath.empty()) {
                logger.Error("CommandParser", "Project path is required for 'repair' command");
                PrintCommandHelp("repair");
                return nullptr;
            }
            return std::make_unique<Commands::RepairCommand>(args.projectPath);
        }

        if (args.commandName == "install-tools" || args.commandName == "bootstrap") {
            if (!NeoPM::Core::ToolchainBootstrap::EnsureInstalled()) {
                logger.Error("CommandParser", "Failed to install NeoWPF toolchain bootstrap");
                return nullptr;
            }

            std::cout << "NeoWPF toolchain bootstrap installed at: "
                      << NeoPM::Core::ToolchainBootstrap::GetTargetsFilePath().string()
                      << std::endl;
            return nullptr;
        }

        logger.Error("CommandParser", "Unknown command: " + args.commandName);
        PrintUsage();
        return nullptr;
    }

    const char* CommandParser::GetVersion() const {
        return "0.1.0";
    }

    void CommandParser::PrintUsage() const {
        std::cout << R"(NeoPM - NeoWPF Project Manager

Usage:
  NeoPM <command> [options]

Commands:
  new <name>                      Create a new NeoWPF project
    daemon                          Run the NeoPM resource daemon
  sync-resources <manifest>       Synchronize resources and generate code
  refresh-sdk                     Download and install latest NeoWPF SDKs
  repair <project-path>           Repair project tooling without deleting project files
  install-tools                   Install LocalAppData NeoWPF build targets (rarely needed)
  bootstrap                       Alias for install-tools
  --help, -h                      Show this help message
  --version, -v                   Show version information

Options:
  --output, -o <path>             Specify generated source output directory
  --copy-to <path>                Copy external resources to this directory as well
  --template, -t <name>           Specify project template
  --project-type <vcxproj|cmake|both>
                                 Select project format (default: vcxproj)
  --sdk <production|user>         Select downloaded production or local user SDK
  --sdk-config <debug|release>    Select SDK configuration (default: debug)
  --sdk-include <path>            User SDK include directory containing NeoWPF
  --sdk-lib <path>                User SDK NeoWPF.lib or its containing directory

Examples:
  NeoPM new MyApp
  NeoPM new MyApp --project-type cmake
  NeoPM new MyApp --output C:\Projects
  NeoPM sync-resources Resources/resources.json --output Generated
  NeoPM refresh-sdk
  NeoPM new --help
)" << std::endl;
    }

    namespace {
        std::string NormalizePathArgument(const std::string& value) {
            auto normalized = value;

            while (!normalized.empty() && (normalized.front() == '"' || normalized.front() == '\'')) {
                normalized.erase(normalized.begin());
            }

            while (!normalized.empty() && (normalized.back() == '"' || normalized.back() == '\'')) {
                normalized.pop_back();
            }

            return normalized;
        }
    }

    CommandArguments CommandParser::ParseArguments(int argc, char* argv[]) {
        CommandArguments args;

        for (int i = 1; i < argc; ++i) {
            auto arg = std::string(argv[i]);

            if (arg == "--help" || arg == "-h") {
                args.showHelp = true;
                if (i > 1) {
                    args.commandName = argv[1];
                }
                continue;
            }

            if (arg == "--version" || arg == "-v") {
                args.showVersion = true;
                continue;
            }

            if (arg == "--background") {
                args.daemonBackground = true;
                continue;
            }

            if (arg == "--output" || arg == "-o") {
                if (i + 1 < argc) {
                    args.outputPath = NormalizePathArgument(argv[++i]);
                }
                continue;
            }

            if (arg == "--copy-to") {
                if (i + 1 < argc) {
                    args.copyOutputPath = NormalizePathArgument(argv[++i]);
                }
                continue;
            }

            if (arg == "--template" || arg == "-t") {
                if (i + 1 < argc) {
                    args.templateName = argv[++i];
                }
                continue;
            }

            if (arg == "--project-type" || arg == "-p") {
                if (i + 1 < argc) {
                    args.projectType = argv[++i];
                    std::transform(
                        args.projectType.begin(),
                        args.projectType.end(),
                        args.projectType.begin(),
                        [](unsigned char character) { return static_cast<char>(std::tolower(character)); }
                    );
                }
                continue;
            }

            if (arg == "--sdk") {
                if (i + 1 < argc) {
                    args.sdkSource = argv[++i];
                }
                continue;
            }

            if (arg == "--sdk-config") {
                if (i + 1 < argc) {
                    args.sdkConfiguration = argv[++i];
                    std::transform(
                        args.sdkConfiguration.begin(),
                        args.sdkConfiguration.end(),
                        args.sdkConfiguration.begin(),
                        [](unsigned char character) { return static_cast<char>(std::tolower(character)); }
                    );
                    args.sdkConfigurationSpecified = true;
                }
                continue;
            }

            if (arg == "--sdk-include") {
                if (i + 1 < argc) {
                    args.sdkIncludePath = NormalizePathArgument(argv[++i]);
                }
                continue;
            }

            if (arg == "--sdk-lib") {
                if (i + 1 < argc) {
                    args.sdkLibraryPath = NormalizePathArgument(argv[++i]);
                }
                continue;
            }

            if (arg[0] != '-') {
                if (args.commandName.empty()) {
                    args.commandName = arg;
                }
                else if (args.projectName.empty() && args.manifestPath.empty() && args.projectPath.empty()) {
                    // for 'new' command, this is the project name
                    //for sync-resources command, this is the manifest path
                    // we'll see which one it is based on commandName when they're set
                    if (args.commandName == "sync-resources") {
                        args.manifestPath = NormalizePathArgument(arg);
                    } else if (args.commandName == "repair") {
                        args.projectPath = NormalizePathArgument(arg);
                    } else {
                        args.projectName = arg;
                    }
                }
            }
        }

        return args;
    }

    void CommandParser::PrintCommandHelp(const std::string& commandName) const {
        if (commandName == "new") {
            std::cout << R"(NeoPM new - Create a new NeoWPF project

Usage:
  NeoPM new <name> [options]

Arguments:
  <name>                  Project name

Options:
  --output, -o <path>             Output directory (default: auto-discover)
  --template, -t <name>           Project template: blank, mvvm (default: blank)
  --project-type <vcxproj|cmake|both>
                                 Project format (default: vcxproj)
  --sdk <production|user>         SDK source (default: production)
  --sdk-config <debug|release>    SDK configuration (default: debug)
  --sdk-include <path>            User SDK include directory containing NeoWPF
  --sdk-lib <path>                User SDK NeoWPF.lib or its containing directory
  --help, -h                      Show this help message

Examples:
  NeoPM new MyApplication
  NeoPM new MyApp --project-type cmake
  NeoPM new MyApp --output D:\Projects
  NeoPM new MyApp --template mvvm
  NeoPM new MyApp --sdk user --sdk-config debug --sdk-include C:\NeoWPF\include --sdk-lib C:\NeoWPF\x64\Debug
)" << std::endl;
            return;
        }

        if (commandName == "sync-resources") {
            std::cout << R"(NeoPM sync-resources - Synchronize resources and generate code

Usage:
  NeoPM sync-resources <manifest> [options]

Arguments:
  <manifest>              Path to resources.json manifest file

Options:
  --output, -o <path>     Output directory for generated files (default: Generated/)
  --help, -h              Show this help message

Examples:
  NeoPM sync-resources Resources/resources.json
  NeoPM sync-resources Resources/resources.json --output Generated
)" << std::endl;
            return;
        }

        if (commandName == "refresh-sdk") {
            std::cout << R"(NeoPM refresh-sdk - Download and install latest NeoWPF SDKs

Usage:
  NeoPM refresh-sdk

This downloads and installs the latest Debug and Release x64 NeoWPF SDKs from GitHub.
The SDKs are extracted to:
  <NeoWPF>/sdk/debug-x64/
  <NeoWPF>/sdk/release-x64/

Examples:
  NeoPM refresh-sdk
)" << std::endl;
            return;
        }

        if (commandName == "repair") {
            std::cout << "NeoPM repair - Repair an existing NeoWPF project\n\n"
                         "Usage:\n"
                         "  NeoPM repair <project-path>\n\n"
                         "Repairs NeoWPF build targets and resource integration without deleting project files.\n"
                      << std::endl;
            return;
        }

        if (commandName == "install-tools" || commandName == "bootstrap") {
            std::cout << R"(NeoPM install-tools - Install the NeoWPF LocalAppData build bootstrap

Usage:
  NeoPM install-tools

This installs the shared MSBuild targets file used by generated projects at:
  %LocalAppData%\NeoWPF\NeoWPF.Resources.targets
)" << std::endl;
            return;
        }

        std::cout << "Unknown command: " << commandName << std::endl;
    }
}
