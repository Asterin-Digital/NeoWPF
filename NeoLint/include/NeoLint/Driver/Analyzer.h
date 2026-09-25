#pragma once

#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleConfig.h>

#include <filesystem>
#include <string>

namespace neolint
{
struct AnalysisOptions
{
    std::filesystem::path compilationDatabase;
    std::filesystem::path config;
};

class Analyzer
{
public:
    int Run(const AnalysisOptions& options, DiagnosticsSink& diagnostics) const;
};
}
