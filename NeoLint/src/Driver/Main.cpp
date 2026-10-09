#include <NeoLint/Driver/Analyzer.h>
#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <charconv>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace
{
void PrintUsage(const char* executable)
{
    std::cout << "Usage: " << executable
              << " --compile-commands <compile_commands.json> [--config <.neolint.json>]"
              << " [--build-config <Debug|Release|RelWithDebInfo|MinSizeRel>]"
              << " [--report <report.json>] [--cache <cache.json>]"
              << " [--source-root <src-or-include>]... [--jobs N] [--strict]\n"
              << "       " << executable << " --list-rules\n";
}
}

int main(int argc, char** argv)
{
    neolint::AnalysisOptions options;
    options.executable = argv[0];
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
        if (argument == "--jobs" || argument == "-j")
        {
            if (index + 1 >= argc)
            {
                std::cerr << "NeoLint: unknown or incomplete argument: " << argument << '\n';
                PrintUsage(argv[0]);
                return EXIT_FAILURE;
            }
            const std::string_view value(argv[++index]);
            std::size_t jobs = 0;
            const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), jobs);
            if (error != std::errc{} || end != value.data() + value.size() || jobs == 0)
            {
                std::cerr << "NeoLint: unknown or incomplete argument: " << argument << '\n';
                PrintUsage(argv[0]);
                return EXIT_FAILURE;
            }
            options.jobs = jobs;
            continue;
        }
        if ((argument == "--compile-commands" || argument == "--config" ||
             argument == "--build-config" || argument == "--report" ||
             argument == "--cache" || argument == "--source-root") && index + 1 < argc)
        {
            if (argument == "--compile-commands") options.compilationDatabase = argv[++index];
            else if (argument == "--config") options.config = argv[++index];
            else if (argument == "--build-config") options.buildConfig = argv[++index];
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
        for (const auto& id : neolint::RuleRegistry::Instance().Ids())
        {
            if (auto rule = neolint::RuleRegistry::Instance().Create(id))
            {
                std::cout << id << " [AST, "
                          << (rule->DefaultSeverity() == neolint::RuleSeverity::Error ?
                              "error" : rule->DefaultSeverity() == neolint::RuleSeverity::Info ?
                                  "info" : "warning")
                          << "]: " << rule->Description() << '\n';
            }
            else if (auto treeRule = neolint::RuleRegistry::Instance().CreateTree(id))
            {
                std::cout << id << " [tree, "
                          << (treeRule->DefaultSeverity() == neolint::RuleSeverity::Error ?
                              "error" : treeRule->DefaultSeverity() == neolint::RuleSeverity::Info ?
                                  "info" : "warning")
                          << "]: " << treeRule->Description() << '\n';
            }
        }
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
