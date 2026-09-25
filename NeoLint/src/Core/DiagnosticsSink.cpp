#include <NeoLint/Core/DiagnosticsSink.h>

#include <iostream>

namespace neolint
{
void DiagnosticsSink::Report(RuleDiagnostic diagnostic)
{
    diagnostics_.push_back(std::move(diagnostic));
}

void DiagnosticsSink::WriteHuman(std::string_view) const
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
