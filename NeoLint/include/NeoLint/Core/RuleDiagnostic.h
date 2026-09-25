#pragma once

#include <NeoLint/Core/RuleSeverity.h>
#include <NeoLint/Core/SourceLocation.h>

#include <string>
#include <vector>

namespace neolint
{
struct RuleDiagnostic
{
    std::string ruleId;
    RuleSeverity severity = RuleSeverity::Info;
    SourceLocation location;
    std::string message;
};
using DiagnosticList = std::vector<RuleDiagnostic>;
}
