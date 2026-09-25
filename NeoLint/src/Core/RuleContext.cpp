#include <NeoLint/Core/RuleContext.h>

namespace neolint
{
RuleContext::RuleContext(clang::ASTContext& ast, const RuleConfig& config,
                         DiagnosticsSink& diagnostics)
    : RuleContext(ast, config, diagnostics, false)
{
}

RuleContext::RuleContext(clang::ASTContext& ast, const RuleConfig& config,
                         DiagnosticsSink& diagnostics, bool strict)
        : ast_(ast), config_(config), diagnostics_(diagnostics),
            suppression_(ast.getSourceManager()), strict_(strict)
{
}

clang::SourceManager& RuleContext::SourceMgr() const noexcept
{
    return ast_.getSourceManager();
}

clang::ASTContext& RuleContext::AST() const noexcept
{
    return ast_;
}

const RuleConfig& RuleContext::Config() const noexcept
{
    return config_;
}

RuleSettings RuleContext::SettingsFor(std::string_view ruleId) const
{
    const auto mainFile = SourceMgr().getLocForStartOfFile(SourceMgr().getMainFileID());
    return config_.SettingsFor(ruleId,
                               std::filesystem::path(SourceMgr().getFilename(mainFile).str()));
}

void RuleContext::Report(std::string_view ruleId, clang::SourceLocation location,
                        RuleSeverity severity, std::string message) const
{
    if (severity == RuleSeverity::Info && !strict_) return;
    if (suppression_.IsSuppressed(ruleId, location)) return;
    const auto& sourceManager = ast_.getSourceManager();
    const auto presumed = sourceManager.getPresumedLoc(location);
    if (presumed.isInvalid()) return;
    diagnostics_.Report({std::string(ruleId), severity,
                         {presumed.getFilename(), presumed.getLine(), presumed.getColumn()},
                         std::move(message)});
}
}
