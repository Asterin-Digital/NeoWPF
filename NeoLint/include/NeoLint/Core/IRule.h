#pragma once

#include <NeoLint/Core/RuleConfig.h>

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <string_view>

namespace neolint
{
class RuleContext;

class IRule
{
public:
    virtual ~IRule() = default;
    virtual std::string_view Id() const noexcept = 0;
    virtual std::string_view Description() const noexcept = 0;
    virtual RuleSeverity DefaultSeverity() const noexcept = 0;
    virtual void RegisterMatchers(
        clang::ast_matchers::MatchFinder& finder,
        clang::ast_matchers::MatchFinder::MatchCallback& callback) = 0;
    virtual void OnMatch(const clang::ast_matchers::MatchFinder::MatchResult& result,
                         RuleContext& context) = 0;
};
}
