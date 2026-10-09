#pragma once

#include <NeoLint/Core/RuleConfig.h>

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <string>
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
    virtual bool ValidateOptions(const RuleOptions& options, std::string& error) const
    {
        if (options.Empty()) return true;
        error = "rule '" + std::string(Id()) + "' does not accept options";
        return false;
    }
    virtual void Configure(const RuleOptions&) {}
    virtual void RegisterMatchers(
        clang::ast_matchers::MatchFinder& finder,
        clang::ast_matchers::MatchFinder::MatchCallback& callback) = 0;
    virtual void OnMatch(const clang::ast_matchers::MatchFinder::MatchResult& result,
                         RuleContext& context) = 0;
};
}
