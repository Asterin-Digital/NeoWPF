#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

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
    const auto key = std::string(ruleId);
    const auto cached = settingsCache_.find(key);
    if (cached != settingsCache_.end()) return cached->second;
    const auto mainFile = SourceMgr().getLocForStartOfFile(SourceMgr().getMainFileID());
    auto settings = config_.SettingsFor(ruleId,
        std::filesystem::path(SourceMgr().getFilename(mainFile).str()));
    if (!config_.HasRule(ruleId))
    {
        if (const auto rule = RuleRegistry::Instance().Create(ruleId))
        {
            settings.severity = rule->DefaultSeverity();
        }
    }
    return settingsCache_.emplace(key, std::move(settings)).first->second;
}

RuleOptions RuleContext::OptionsFor(std::string_view ruleId) const
{
    return SettingsFor(ruleId).options;
}

void RuleContext::Report(std::string_view ruleId, clang::SourceLocation location,
                        RuleSeverity severity, std::string message) const
{
    const auto& sourceManager = ast_.getSourceManager();
    const auto presumed = sourceManager.getPresumedLoc(location);
    if (presumed.isInvalid()) return;
    const auto settings = config_.SettingsFor(ruleId, presumed.getFilename());
    if (!settings.enabled) return;
    if (config_.HasRule(ruleId)) severity = settings.severity;
    if (severity == RuleSeverity::Info && !strict_) return;
    if (suppression_.IsSuppressed(ruleId, location)) return;
    diagnostics_.Report({std::string(ruleId), severity,
                         {presumed.getFilename(), presumed.getLine(), presumed.getColumn()},
                         std::move(message)});
}
}
