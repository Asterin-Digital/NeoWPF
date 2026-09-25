#include <NeoLint/Core/RuleContext.h>

namespace neolint
{
RuleContext::RuleContext(clang::ASTContext& ast, const RuleConfig& config,
                         DiagnosticsSink& diagnostics)
    : ast_(ast), config_(config), diagnostics_(diagnostics)
{
}

clang::ASTContext& RuleContext::AST() const noexcept
{
    return ast_;
}

const RuleConfig& RuleContext::Config() const noexcept
{
    return config_;
}

void RuleContext::Report(std::string_view ruleId, clang::SourceLocation location,
                        RuleSeverity severity, std::string message) const
{
    const auto& sourceManager = ast_.getSourceManager();
    const auto presumed = sourceManager.getPresumedLoc(location);
    if (presumed.isInvalid()) return;
    diagnostics_.Report({std::string(ruleId), severity,
                         {presumed.getFilename(), presumed.getLine(), presumed.getColumn()},
                         std::move(message)});
}
}
