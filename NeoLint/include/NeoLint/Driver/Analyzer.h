#pragma once

#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleConfig.h>

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
    std::vector<std::filesystem::path> sourceRoots;
    std::string buildConfig;
    bool strict = false;
};

class Analyzer
{
public:
    int Run(const AnalysisOptions& options, DiagnosticsSink& diagnostics) const;
};
}
