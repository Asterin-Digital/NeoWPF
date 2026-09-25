#include <NeoLint/Driver/Analyzer.h>
#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void PrintUsage(const char* executable)
{
    std::cout << "Usage: " << executable
              << " --compile-commands <compile_commands.json> [--config <.neolint.yml>]\n"
              << "       " << executable << " --list-rules\n";
}
}

int main(int argc, char** argv)
{
    neolint::AnalysisOptions options;
    bool listRules = false;
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--help" || argument == "-h")
        {
            PrintUsage(argv[0]);
            return EXIT_SUCCESS;
        }
        if (argument == "--list-rules")
        {
            listRules = true;
            continue;
        }
        if ((argument == "--compile-commands" || argument == "--config") && index + 1 < argc)
        {
            if (argument == "--compile-commands") options.compilationDatabase = argv[++index];
            else options.config = argv[++index];
            continue;
        }
        std::cerr << "NeoLint: unknown or incomplete argument: " << argument << '\n';
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }

    if (listRules)
    {
        for (const auto& id : neolint::RuleRegistry::Instance().Ids()) std::cout << id << '\n';
        return EXIT_SUCCESS;
    }
    if (options.compilationDatabase.empty())
    {
        std::cerr << "NeoLint: --compile-commands is required\n";
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }

    neolint::DiagnosticsSink diagnostics;
    return neolint::Analyzer{}.Run(options, diagnostics);
}
