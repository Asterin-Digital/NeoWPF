#pragma once

#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleConfig.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace neolint
{
struct AnalysisOptions
{
    std::filesystem::path compilationDatabase;
    std::filesystem::path config;
    std::filesystem::path report;
    std::filesystem::path cache;
    std::filesystem::path executable;
    std::vector<std::filesystem::path> sourceRoots;
    std::string buildConfig;
    std::size_t jobs = 0;
    bool strict = false;
};

class Analyzer
{
public:
    int Run(const AnalysisOptions& options, DiagnosticsSink& diagnostics) const;
};
}
