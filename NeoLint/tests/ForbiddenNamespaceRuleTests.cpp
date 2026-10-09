#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Rules/ForbiddenNamespaceRule.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include <gtest/gtest.h>

#include <memory>
#include <string>

namespace
{
class RuleAdapter final : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    RuleAdapter(neolint::ForbiddenNamespaceRule& rule, neolint::RuleContext& context)
        : rule_(rule), context_(context) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        rule_.OnMatch(result, context_);
    }

private:
    neolint::ForbiddenNamespaceRule& rule_;
    neolint::RuleContext& context_;
};

class RuleConsumer final : public clang::ASTConsumer
{
public:
    RuleConsumer(clang::ASTContext& ast, neolint::DiagnosticsSink& diagnostics)
        : context_(ast, config_, diagnostics), adapter_(rule_, context_)
    {
        rule_.RegisterMatchers(finder_, adapter_);
    }

    void HandleTranslationUnit(clang::ASTContext& ast) override
    {
        finder_.matchAST(ast);
    }

private:
    neolint::RuleConfig config_;
    neolint::RuleContext context_;
    neolint::ForbiddenNamespaceRule rule_;
    RuleAdapter adapter_;
    clang::ast_matchers::MatchFinder finder_;
};

class RuleAction final : public clang::ASTFrontendAction
{
public:
    explicit RuleAction(neolint::DiagnosticsSink& diagnostics) : diagnostics_(diagnostics) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler, llvm::StringRef) override
    {
        return std::make_unique<RuleConsumer>(compiler.getASTContext(), diagnostics_);
    }

private:
    neolint::DiagnosticsSink& diagnostics_;
};
}

TEST(ForbiddenNamespaceRuleTests, ReportsForbiddenComponentsOnlyUnderNeoWPF)
{
    const std::string code = R"(
namespace NeoWPF::UI::DetailApi {}
namespace NeoWPF::ImplementationSupport {}
namespace NeoWPF::PrivateHelpers {}
namespace NeoWPF::InternalAPI {}
namespace NeoWPF::Public::Helpers {}
namespace Other::Internal {}
)";
    neolint::DiagnosticsSink diagnostics;

    ASSERT_TRUE(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<RuleAction>(diagnostics), code, {"-std=c++20"}));
    ASSERT_EQ(diagnostics.Diagnostics().size(), 4u);
    for (const auto& diagnostic : diagnostics.Diagnostics())
    {
        EXPECT_EQ(diagnostic.ruleId, "neowpf-forbidden-namespace");
        EXPECT_EQ(diagnostic.severity, neolint::RuleSeverity::Error);
        EXPECT_NE(diagnostic.location.file, "");
    }
}
