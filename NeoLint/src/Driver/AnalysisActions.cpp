#include <NeoLint/Driver/AnalysisActions.h>

#include <NeoLint/Core/IRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>

#include <filesystem>
#include <algorithm>
#include <memory>
#include <vector>

namespace neolint::driver
{
namespace
{
bool IsUnder(const std::filesystem::path& file, const std::filesystem::path& root)
{
    const auto relative = file.lexically_relative(root);
    return !relative.empty() && relative != "." &&
        *relative.begin() != ".." && !relative.is_absolute();
}

class DependencyCollector final : public clang::PPCallbacks
{
public:
    DependencyCollector(clang::SourceManager& sourceManager, DependencyPaths& dependencies)
        : sourceManager_(sourceManager), dependencies_(dependencies) {}

    void LexedFileChanged(clang::FileID fileId, LexedFileChangeReason reason,
                          clang::SrcMgr::CharacteristicKind, clang::FileID,
                          clang::SourceLocation) override
    {
        if (reason != LexedFileChangeReason::EnterFile) return;
        const auto location = sourceManager_.getLocForStartOfFile(fileId);
        if (location.isInvalid() || sourceManager_.isInSystemHeader(location)) return;
        const auto filename = sourceManager_.getFilename(location);
        if (!filename.empty()) dependencies_.emplace_back(filename.str());
    }

private:
    clang::SourceManager& sourceManager_;
    DependencyPaths& dependencies_;
};

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
           DiagnosticsSink& diagnostics, bool strict,
           const std::vector<std::filesystem::path>& sourceRoots)
       : context_(ast, config, diagnostics, strict), sourceRoots_(sourceRoots)
    {
        for (const auto& id : RuleRegistry::Instance().Ids())
        {
            auto rule = RuleRegistry::Instance().Create(id);
            if (!rule || !context_.SettingsFor(id).enabled) continue;
            rule->Configure(context_.OptionsFor(id));
            adapters_.push_back(std::make_unique<MatchAdapter>(*rule, context_));
            rules_.push_back(std::move(rule));
            rules_.back()->RegisterMatchers(finder_, *adapters_.back());
        }
    }

    void HandleTranslationUnit(clang::ASTContext& ast) override
    {
        if (!sourceRoots_.empty())
        {
            std::vector<clang::Decl*> scope;
            auto& sourceManager = context_.SourceMgr();
            for (auto* declaration : ast.getTranslationUnitDecl()->decls())
            {
                const auto location = declaration->getLocation();
                if (location.isInvalid()) continue;
                std::error_code error;
                auto filePath = std::filesystem::weakly_canonical(
                    sourceManager.getFilename(location).str(), error);
                if (error) filePath = std::filesystem::absolute(
                    sourceManager.getFilename(location).str());
                if (std::any_of(sourceRoots_.begin(), sourceRoots_.end(),
                    [&](const auto& root) { return IsUnder(filePath, root); }))
                {
                    scope.push_back(declaration);
                }
            }
            ast.setTraversalScope(scope);
        }
        finder_.matchAST(context_.AST());
    }

private:
    RuleContext context_;
    const std::vector<std::filesystem::path>& sourceRoots_;
    clang::ast_matchers::MatchFinder finder_;
    std::vector<std::unique_ptr<IRule>> rules_;
    std::vector<std::unique_ptr<MatchAdapter>> adapters_;
};

class Action final : public clang::ASTFrontendAction
{
public:
    Action(const RuleConfig& config, DiagnosticsSink& diagnostics, bool strict,
           const std::vector<std::filesystem::path>& sourceRoots,
           DependencyPaths& dependencies)
        : config_(config), diagnostics_(diagnostics), strict_(strict),
          sourceRoots_(sourceRoots), dependencies_(dependencies) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler, llvm::StringRef) override
    {
        compiler.getPreprocessor().addPPCallbacks(
            std::make_unique<DependencyCollector>(compiler.getSourceManager(), dependencies_));
        return std::make_unique<Consumer>(compiler.getASTContext(), config_, diagnostics_, strict_,
                                          sourceRoots_);
    }

private:
    const RuleConfig& config_;
    DiagnosticsSink& diagnostics_;
    bool strict_;
        const std::vector<std::filesystem::path>& sourceRoots_;
        DependencyPaths& dependencies_;
};
}

ActionFactory::ActionFactory(const RuleConfig& config, DiagnosticsSink& diagnostics, bool strict,
                                                         std::vector<std::filesystem::path> sourceRoots,
                                                         DependencyPaths& dependencies)
        : config_(config), diagnostics_(diagnostics), strict_(strict),
            sourceRoots_(std::move(sourceRoots)), dependencies_(dependencies) {}

std::unique_ptr<clang::FrontendAction> ActionFactory::create()
{
    return std::make_unique<Action>(config_, diagnostics_, strict_, sourceRoots_, dependencies_);
}
}
