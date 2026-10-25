/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoLint/Core/IRule.h>

#include <clang/Basic/SourceManager.h>
#include <clang/Lex/PPCallbacks.h>

#include <memory>

namespace neolint
{
class DiagnosticsSink;

class PlatformDependencyIsolationRule final : public IRule
{
public:
    std::string_view Id() const noexcept override;
    std::string_view Description() const noexcept override;
    RuleSeverity DefaultSeverity() const noexcept override;
    bool ValidateOptions(const RuleOptions& options, std::string& error) const override;
    void Configure(const RuleOptions& options) override;
    void RegisterMatchers(
        clang::ast_matchers::MatchFinder& finder,
        clang::ast_matchers::MatchFinder::MatchCallback& callback) override;
    void OnMatch(const clang::ast_matchers::MatchFinder::MatchResult& result,
                 RuleContext& context) override;

    static std::unique_ptr<clang::PPCallbacks> CreatePreprocessorCallbacks(
        const RuleOptions& options, const RuleConfig& config, DiagnosticsSink& diagnostics,
        clang::SourceManager& sourceManager, bool strict);

private:
    RuleOptions options_;
};
}