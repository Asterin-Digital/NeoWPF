#include <NeoLint/Driver/AnalysisActions.h>

#include <NeoLint/Core/IRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>

#include <memory>
#include <vector>

namespace neolint::driver
{
namespace
{
class MatchAdapter final : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    MatchAdapter(IRule& rule, RuleContext& context) : rule_(rule), context_(context) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        rule_.OnMatch(result, context_);
    }

private:
    IRule& rule_;
    RuleContext& context_;
};

class Consumer final : public clang::ASTConsumer
{
public:
    Consumer(clang::ASTContext& ast, const RuleConfig& config,
             DiagnosticsSink& diagnostics, bool strict)
        : context_(ast, config, diagnostics, strict)
    {
        for (const auto& id : RuleRegistry::Instance().Ids())
        {
            auto rule = RuleRegistry::Instance().Create(id);
            if (!rule || !context_.SettingsFor(id).enabled) continue;
            adapters_.push_back(std::make_unique<MatchAdapter>(*rule, context_));
            rules_.push_back(std::move(rule));
            rules_.back()->RegisterMatchers(finder_, *adapters_.back());
        }
    }

    void HandleTranslationUnit(clang::ASTContext&) override
    {
        finder_.matchAST(context_.AST());
    }

private:
    RuleContext context_;
    clang::ast_matchers::MatchFinder finder_;
    std::vector<std::unique_ptr<IRule>> rules_;
    std::vector<std::unique_ptr<MatchAdapter>> adapters_;
};

class Action final : public clang::ASTFrontendAction
{
public:
    Action(const RuleConfig& config, DiagnosticsSink& diagnostics, bool strict)
        : config_(config), diagnostics_(diagnostics), strict_(strict) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler, llvm::StringRef) override
    {
        return std::make_unique<Consumer>(compiler.getASTContext(), config_, diagnostics_, strict_);
    }

private:
    const RuleConfig& config_;
    DiagnosticsSink& diagnostics_;
    bool strict_;
};
}

ActionFactory::ActionFactory(const RuleConfig& config, DiagnosticsSink& diagnostics, bool strict)
    : config_(config), diagnostics_(diagnostics), strict_(strict) {}

std::unique_ptr<clang::FrontendAction> ActionFactory::create()
{
    return std::make_unique<Action>(config_, diagnostics_, strict_);
}
}
