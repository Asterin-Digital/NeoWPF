#pragma once

#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleConfig.h>

#include <clang/Tooling/Tooling.h>

#include <memory>

namespace neolint::driver
{
class ActionFactory final : public clang::tooling::FrontendActionFactory
{
public:
    ActionFactory(const RuleConfig& config, DiagnosticsSink& diagnostics, bool strict);

    std::unique_ptr<clang::FrontendAction> create() override;

private:
    const RuleConfig& config_;
    DiagnosticsSink& diagnostics_;
    bool strict_;
};
}
