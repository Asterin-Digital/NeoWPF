/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoLint/Core/IRule.h>
#include <NeoLint/Core/ITreeRule.h>

namespace neolint
{
class PlatformCodeLocationRule final : public IRule, public ITreeRule
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
    void Run(const std::vector<std::filesystem::path>& sourceRoots,
             const RuleConfig& config, DiagnosticsSink& diagnostics) override;

private:
    RuleOptions options_;
};
}