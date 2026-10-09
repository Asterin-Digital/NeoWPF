#pragma once

#include <NeoLint/Core/IRule.h>

#include <map>
#include <string>
#include <vector>

namespace neolint
{
class PlatformUtilityDuplicateRule final : public IRule
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
    std::map<std::string, std::string> aliases_;
    std::vector<std::string> ignoredNames_;
    std::string canonicalNamespace_ = "NeoWPF::Platform";
};
}
