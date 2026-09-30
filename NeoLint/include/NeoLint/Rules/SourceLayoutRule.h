#pragma once

#include <NeoLint/Core/IRule.h>

namespace neolint
{
class SourceLayoutRule final : public IRule
{
public:
    std::string_view Id() const noexcept override;
    std::string_view Description() const noexcept override;
    RuleSeverity DefaultSeverity() const noexcept override;
    void RegisterMatchers(
        clang::ast_matchers::MatchFinder& finder,
        clang::ast_matchers::MatchFinder::MatchCallback& callback) override;
    void OnMatch(const clang::ast_matchers::MatchFinder::MatchResult& result,
                 RuleContext& context) override;
};
}
