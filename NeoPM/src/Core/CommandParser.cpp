#include <NeoPM/Core/CommandParser.h>
#include <NeoPM/Core/Daemon/DaemonCommand.h>
#include <NeoPM/Core/Commands/NewCommand.h>
#include <NeoPM/Core/Commands/SyncResourcesCommand.h>
#include <NeoPM/Core/Commands/RefreshSdkCommand.h>
#include <NeoPM/Core/Commands/RepairCommand.h>
#include <NeoPM/Core/ToolchainBootstrap.h>
#include <NeoPM/Utils/Logger.h>
#include <NeoPM/Utils/TextEncoding.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>
#include <string_view>

namespace NeoPM::Core {
        namespace {
                struct CommandHelp {
                        std::string_view name;
                        std::string_view text;
                };

                constexpr std::string_view UsageText = R"(NeoPM - NeoWPF Project Manager

Usage:
    NeoPM <command> [options]

Commands:
    new <name>                      Create a new NeoWPF project
    daemon                          Run the NeoPM resource daemon
    sync-resources <manifest>       Synchronize resources and generate code
    refresh-sdk                     Download and install latest NeoWPF SDKs
    repair <project-path>           Repair project tooling without deleting project files
    install-tools                   Install the NeoPM resource sync helper
    bootstrap                       Alias for install-tools
    help [command]                  Show command help
    --help, -h                      Show this help message
    --version, -v                   Show version information

Options:
    --output, -o <path>             Specify generated source output directory
    --copy-to <path>                Copy external resources to this directory as well
    --template, -t <name>           Specify project template
    --sdk <production|user>         Select downloaded production or local user SDK
    --sdk-config <debug|release>    Select SDK configuration (default: debug)

Examples:
    NeoPM new MyApp
    NeoPM new MyApp --output C:\Projects
    NeoPM sync-resources Resources/resources.json --output Generated
    NeoPM refresh-sdk
    NeoPM help new
)";

                constexpr std::array<CommandHelp, 5> CommandHelpTexts{{
                        {"new", R"(NeoPM new - Create a new NeoWPF project

Usage:
    NeoPM new <name> [options]

Arguments:
    <name>                  Project name

Project files:
    CMakeLists.txt and CMakePresets.json

Options:
    --output, -o <path>             Output directory (default: auto-discover)
    --template, -t <name>           Project template: blank, mvvm (default: blank)
    --sdk <production|user>         SDK source (default: production)
    --sdk-config <debug|release>    SDK configuration (default: debug)
    --sdk-include-dir <path>        User SDK include directory (repeatable)
    --sdk-lib <path>                User SDK library root with Debug and Release subdirectories
    --help, -h                      Show this help message

Examples:
    NeoPM new MyApplication
    NeoPM new MyApp --output D:\Projects
    NeoPM new MyApp --template mvvm
    NeoPM new MyApp --sdk user --sdk-config debug --sdk-include-dir C:\NeoWPF\include --sdk-include-dir C:\NeoWPF\third_party\include --sdk-lib C:\NeoWPF\x64
)"},
                        {"sync-resources", R"(NeoPM sync-resources - Synchronize resources and generate code

Usage:
    NeoPM sync-resources <manifest> [options]

Arguments:
    <manifest>              Path to resources.json manifest file

Options:
    --output, -o <path>     Output directory for generated files (default: Generated/)
    --copy-to <path>        Copy external resources to this directory as well
    --help, -h              Show this help message

Examples:
    NeoPM sync-resources Resources/resources.json
    NeoPM sync-resources Resources/resources.json --output Generated
)"},
                        {"refresh-sdk", R"(NeoPM refresh-sdk - Download and install latest NeoWPF SDKs

Usage:
    NeoPM refresh-sdk

This downloads and installs the supported platform SDKs from GitHub.
The SDKs are stored under:
    %LocalAppData%\Asterin Digital\SDK\<platform>-<architecture>\<version>\

Examples:
    NeoPM refresh-sdk
)"},
                        {"repair", R"(NeoPM repair - Repair an existing NeoWPF project

Usage:
    NeoPM repair <project-path>

Repairs NeoWPF build targets and resource integration without deleting project files.
)"},
                        {"install-tools", R"(NeoPM install-tools - Install the resource sync helper

Usage:
    NeoPM install-tools

Installs the CMake resource sync module at:
    %LocalAppData%\Asterin Digital\NeoWPFResources.cmake
)"}
                }};

                bool IsKnownCommand(std::string_view command)
                {
                        return command == "new" || command == "daemon" || command == "sync-resources" ||
                                command == "refresh-sdk" || command == "repair" || command == "install-tools" ||
                                command == "bootstrap";
                }

                bool RequiresToolchainBootstrap(std::string_view command)
                {
                        return command == "new" || command == "daemon" || command == "sync-resources";
                }
        }

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

        if (args.commandName == "help") {
            if (args.projectName.empty()) {
                PrintUsage();
            }
            else {
                PrintCommandHelp(args.projectName);
            }
            return nullptr;
        }

        if (args.invalidOption) {
            logger.Error("CommandParser", "Unknown option: " + *args.invalidOption);
            if (!args.commandName.empty()) {
                PrintCommandHelp(args.commandName);
            } else {
                PrintUsage();
            }
            return nullptr;
        }

        if (!IsKnownCommand(args.commandName)) {
            if (args.commandName.empty()) {
                PrintUsage();
            } else {
                logger.Error("CommandParser", "Unknown command: " + args.commandName);
                PrintUsage();
            }
            return nullptr;
        }

        if (RequiresToolchainBootstrap(args.commandName)) {
            if (!NeoPM::Core::ToolchainBootstrap::EnsureInstalled()) {
                logger.Error("CommandParser", "Failed to install the Asterin Digital resource sync helper");
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
                (!args.sdkConfigurationSpecified || args.sdkIncludeDirectories.empty() || !args.sdkLibraryPath)) {
                logger.Error(
                    "CommandParser",
                    "User SDKs require --sdk-config, at least one --sdk-include-dir, and --sdk-lib"
                );
                PrintCommandHelp("new");
                return nullptr;
            }
            return std::make_unique<Commands::NewCommand>(
                args.projectName,
                args.outputPath,
                args.sdkSource,
                args.sdkConfiguration,
                args.sdkIncludeDirectories,
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
                      << NeoPM::Core::ToolchainBootstrap::GetCMakeModulePath().string()
                      << std::endl;
            return nullptr;
        }

        return nullptr;
    }

    const char* CommandParser::GetVersion() const {
        return "0.1.0";
    }

    void CommandParser::PrintUsage() const {
                std::cout << UsageText;
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
                if (i > 1 && argv[1][0] != '-') {
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

            if (arg == "--sdk-include-dir") {
                if (i + 1 < argc) {
                    args.sdkIncludeDirectories.push_back(NormalizePathArgument(argv[++i]));
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
            } else {
                args.invalidOption = arg;
            }
        }

        return args;
    }

    void CommandParser::PrintCommandHelp(const std::string& commandName) const {
                const auto help = std::find_if(CommandHelpTexts.begin(), CommandHelpTexts.end(),
                        [&commandName](const auto& entry) {
                                return entry.name == commandName ||
                                        (commandName == "bootstrap" && entry.name == "install-tools");
                        });
                if (help != CommandHelpTexts.end()) {
                        std::cout << help->text;
                        return;
                }

                std::cout << "Unknown help topic: " << commandName << "\n\n";
                PrintUsage();
    }
}
