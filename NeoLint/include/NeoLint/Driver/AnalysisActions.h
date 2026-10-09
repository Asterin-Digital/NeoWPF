#pragma once

#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleConfig.h>

#include <clang/Tooling/Tooling.h>

#include <filesystem>
#include <memory>
#include <vector>

namespace neolint::driver
{
using DependencyPaths = std::vector<std::filesystem::path>;

class ActionFactory final : public clang::tooling::FrontendActionFactory
{
public:
    ActionFactory(const RuleConfig& config, DiagnosticsSink& diagnostics, bool strict,
                  std::vector<std::filesystem::path> sourceRoots,
                  DependencyPaths& dependencies);

    std::unique_ptr<clang::FrontendAction> create() override;

private:
    const RuleConfig& config_;
    DiagnosticsSink& diagnostics_;
    bool strict_;
    std::vector<std::filesystem::path> sourceRoots_;
    DependencyPaths& dependencies_;
};
}
