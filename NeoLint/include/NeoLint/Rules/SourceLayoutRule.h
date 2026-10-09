#pragma once

#include <NeoLint/Core/ITreeRule.h>

namespace neolint
{
class SourceLayoutRule final : public ITreeRule
{
public:
    std::string_view Id() const noexcept override;
    std::string_view Description() const noexcept override;
    RuleSeverity DefaultSeverity() const noexcept override;
    bool ValidateOptions(const RuleOptions& options, std::string& error) const override;
    void Run(const std::vector<std::filesystem::path>& sourceRoots,
             const RuleConfig& config, DiagnosticsSink& diagnostics) override;
};
}
