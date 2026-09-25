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
              << " --compile-commands <compile_commands.json> [--config <.neolint.json>]"
              << " [--report <report.json>] [--cache <cache.json>]"
              << " [--source-root <src-or-include>]... [--strict]\n"
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
        if (argument == "--strict")
        {
            options.strict = true;
            continue;
        }
           if ((argument == "--compile-commands" || argument == "--config" ||
               argument == "--report" || argument == "--cache" ||
               argument == "--source-root") && index + 1 < argc)
        {
            if (argument == "--compile-commands") options.compilationDatabase = argv[++index];
            else if (argument == "--config") options.config = argv[++index];
              else if (argument == "--report") options.report = argv[++index];
              else if (argument == "--cache") options.cache = argv[++index];
              else options.sourceRoots.emplace_back(argv[++index]);
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
