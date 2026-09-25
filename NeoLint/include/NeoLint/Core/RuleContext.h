#pragma once

#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleConfig.h>

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

namespace neolint
{
class RuleContext
{
public:
    RuleContext(clang::ASTContext& ast, const RuleConfig& config, DiagnosticsSink& diagnostics);
    clang::ASTContext& AST() const noexcept;
    const RuleConfig& Config() const noexcept;
    void Report(std::string_view ruleId, clang::SourceLocation location,
                RuleSeverity severity, std::string message) const;

private:
    clang::ASTContext& ast_;
    const RuleConfig& config_;
    DiagnosticsSink& diagnostics_;
};
}
