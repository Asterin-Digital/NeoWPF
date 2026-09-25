#include <NeoLint/Driver/Analyzer.h>
#include <NeoLint/Core/IRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <memory>

namespace neolint
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
    Consumer(clang::ASTContext& ast, const RuleConfig& config, DiagnosticsSink& diagnostics)
        : context_(ast, config, diagnostics)
    {
        for (const auto& id : RuleRegistry::Instance().Ids())
        {
            auto rule = RuleRegistry::Instance().Create(id);
            if (!rule || !config.SettingsFor(id).enabled) continue;
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
    Action(const RuleConfig& config, DiagnosticsSink& diagnostics)
        : config_(config), diagnostics_(diagnostics) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler, llvm::StringRef) override
    {
        return std::make_unique<Consumer>(compiler.getASTContext(), config_, diagnostics_);
    }

private:
    const RuleConfig& config_;
    DiagnosticsSink& diagnostics_;
};

class ActionFactory final : public clang::tooling::FrontendActionFactory
{
public:
    ActionFactory(const RuleConfig& config, DiagnosticsSink& diagnostics)
        : config_(config), diagnostics_(diagnostics) {}

    std::unique_ptr<clang::FrontendAction> create() override
    {
        return std::make_unique<Action>(config_, diagnostics_);
    }

private:
    const RuleConfig& config_;
    DiagnosticsSink& diagnostics_;
};
}

int Analyzer::Run(const AnalysisOptions& options, DiagnosticsSink& diagnostics) const
{
    std::string error;
    const auto config = RuleConfig::Load(options.config, error);
    if (!error.empty()) return 2;

    auto database = clang::tooling::JSONCompilationDatabase::loadFromFile(
        options.compilationDatabase.string(), error,
        clang::tooling::JSONCommandLineSyntax::AutoDetect);
    if (!database) return 2;

    const auto files = database->getAllFiles();
    if (files.empty()) return 2;

    clang::tooling::ClangTool tool(*database, files);
    ActionFactory factory(config, diagnostics);
    const auto result = tool.run(&factory);
    diagnostics.WriteHuman("NeoLint");
    return result == 0 && !diagnostics.HasErrors() ? 0 : 1;
}
}
