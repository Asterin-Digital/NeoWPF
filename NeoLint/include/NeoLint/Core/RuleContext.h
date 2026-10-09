#pragma once

#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleConfig.h>
#include <NeoLint/Core/Suppression.h>

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

#include <unordered_map>

namespace neolint
{
class RuleContext
{
public:
    RuleContext(clang::ASTContext& ast, const RuleConfig& config, DiagnosticsSink& diagnostics);
    RuleContext(clang::ASTContext& ast, const RuleConfig& config,
                DiagnosticsSink& diagnostics, bool strict);
    clang::ASTContext& AST() const noexcept;
    clang::SourceManager& SourceMgr() const noexcept;
    const RuleConfig& Config() const noexcept;
    RuleSettings SettingsFor(std::string_view ruleId) const;
    RuleOptions OptionsFor(std::string_view ruleId) const;
    void Report(std::string_view ruleId, clang::SourceLocation location,
                RuleSeverity severity, std::string message) const;

private:
    clang::ASTContext& ast_;
    const RuleConfig& config_;
    DiagnosticsSink& diagnostics_;
    Suppression suppression_;
    bool strict_ = false;
    mutable std::unordered_map<std::string, RuleSettings> settingsCache_;
};
}
