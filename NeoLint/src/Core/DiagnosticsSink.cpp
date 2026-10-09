#include <NeoLint/Core/DiagnosticsSink.h>

#include <algorithm>
#include <iostream>
#include <set>
#include <tuple>

namespace neolint
{
void DiagnosticsSink::Report(RuleDiagnostic diagnostic)
{
    diagnostics_.push_back(std::move(diagnostic));
}

void DiagnosticsSink::SortAndDeduplicate()
{
    std::set<std::tuple<std::string, std::string, unsigned, unsigned, std::string>> seen;
    diagnostics_.erase(std::remove_if(diagnostics_.begin(), diagnostics_.end(),
        [&](const RuleDiagnostic& diagnostic) {
            return !seen.emplace(diagnostic.ruleId, diagnostic.location.file,
                                 diagnostic.location.line, diagnostic.location.column,
                                 diagnostic.message).second;
        }), diagnostics_.end());
    std::stable_sort(diagnostics_.begin(), diagnostics_.end(),
        [](const RuleDiagnostic& left, const RuleDiagnostic& right) {
            return std::tie(left.location.file, left.location.line, left.location.column,
                            left.ruleId, left.message, left.severity) <
                   std::tie(right.location.file, right.location.line, right.location.column,
                            right.ruleId, right.message, right.severity);
        });
}

void DiagnosticsSink::WriteHuman() const
{
    for (const auto& diagnostic : diagnostics_)
    {
        const char* severity = diagnostic.severity == RuleSeverity::Error ? "error" :
            diagnostic.severity == RuleSeverity::Warning ? "warning" : "info";
        std::cerr << diagnostic.location.file << ':' << diagnostic.location.line << ':'
                  << diagnostic.location.column << ": " << severity << ": ["
                  << diagnostic.ruleId << "] " << diagnostic.message << '\n';
    }
}

bool DiagnosticsSink::HasErrors() const noexcept
{
    for (const auto& diagnostic : diagnostics_)
    {
        if (diagnostic.severity == RuleSeverity::Error) return true;
    }
    return false;
}

const DiagnosticList& DiagnosticsSink::Diagnostics() const noexcept
{
    return diagnostics_;
}
}
