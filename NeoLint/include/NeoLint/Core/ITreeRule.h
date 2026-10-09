/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleConfig.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace neolint
{
class ITreeRule
{
public:
    virtual ~ITreeRule() = default;
    virtual std::string_view Id() const noexcept = 0;
    virtual std::string_view Description() const noexcept = 0;
    virtual RuleSeverity DefaultSeverity() const noexcept = 0;
    virtual bool ValidateOptions(const RuleOptions& options, std::string& error) const
    {
        if (options.Empty()) return true;
        error = "rule '" + std::string(Id()) + "' does not accept options";
        return false;
    }
    virtual void Run(const std::vector<std::filesystem::path>& sourceRoots,
                     const RuleConfig& config, DiagnosticsSink& diagnostics) = 0;
};
}