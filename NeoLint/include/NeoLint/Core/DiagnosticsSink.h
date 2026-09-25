#pragma once

#include <NeoLint/Core/RuleDiagnostic.h>

#include <string_view>

namespace neolint
{
class DiagnosticsSink
{
public:
    void Report(RuleDiagnostic diagnostic);
    void WriteHuman(std::string_view programName) const;
    bool HasErrors() const noexcept;
    const DiagnosticList& Diagnostics() const noexcept;

private:
    DiagnosticList diagnostics_;
};
}
