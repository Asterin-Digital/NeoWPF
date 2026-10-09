#pragma once

#include <NeoLint/Core/IRule.h>

#include <cstddef>
#include <string>
#include <vector>

namespace neolint
{
class PropertySourceTagRule final : public IRule
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

private:
    std::vector<std::string> wrapperMethodNames_{
        "SetTriggerPropertyValue", "SetStylePropertyValue"};
    std::string checkedCallName_ = "SetNamedPropertyValue";
    std::string forbiddenEnumerator_ = "Local";
    std::size_t argumentIndex_ = 2;
};
}
