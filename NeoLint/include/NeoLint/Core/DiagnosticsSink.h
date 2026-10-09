#pragma once

#include <NeoLint/Core/RuleDiagnostic.h>

#include <string_view>

namespace neolint
{
class DiagnosticsSink
{
public:
    void Report(RuleDiagnostic diagnostic);
    /*
        @internal
    */
    void SortAndDeduplicate();
    void WriteHuman() const;
    bool HasErrors() const noexcept;
    const DiagnosticList& Diagnostics() const noexcept;

private:
    DiagnosticList diagnostics_;
};
}
