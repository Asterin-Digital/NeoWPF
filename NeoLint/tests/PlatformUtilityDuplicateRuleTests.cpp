#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleConfig.h>
#include <NeoLint/Rules/PlatformUtilityDuplicateRule.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

namespace
{
class MatchAdapter final : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    MatchAdapter(neolint::PlatformUtilityDuplicateRule& rule,
                 neolint::RuleContext& context)
        : rule_(rule), context_(context) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        rule_.OnMatch(result, context_);
    }

private:
    neolint::PlatformUtilityDuplicateRule& rule_;
    neolint::RuleContext& context_;
};

class Consumer final : public clang::ASTConsumer
{
public:
    Consumer(clang::ASTContext& ast, const neolint::RuleConfig& config,
             neolint::DiagnosticsSink& diagnostics)
        : context_(ast, config, diagnostics), adapter_(rule_, context_)
    {
        rule_.Configure(context_.OptionsFor(rule_.Id()));
        if (context_.SettingsFor(rule_.Id()).enabled)
        {
            rule_.RegisterMatchers(finder_, adapter_);
        }
    }

    void HandleTranslationUnit(clang::ASTContext&) override
    {
        finder_.matchAST(context_.AST());
    }

private:
    neolint::PlatformUtilityDuplicateRule rule_;
    neolint::RuleContext context_;
    MatchAdapter adapter_;
    clang::ast_matchers::MatchFinder finder_;
};

class Action final : public clang::ASTFrontendAction
{
public:
    Action(const neolint::RuleConfig& config, neolint::DiagnosticsSink& diagnostics)
        : config_(config), diagnostics_(diagnostics) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler, llvm::StringRef) override
    {
        return std::make_unique<Consumer>(compiler.getASTContext(), config_, diagnostics_);
    }

private:
    const neolint::RuleConfig& config_;
    neolint::DiagnosticsSink& diagnostics_;
};

neolint::DiagnosticList AnalyzeFixture(const char* fixture)
{
    const auto path = std::filesystem::path(NEOLINT_TEST_SOURCE_DIR) /
        "tests/fixtures/platform_utility_duplicate" / fixture;
    std::ifstream input(path);
    const std::string code((std::istreambuf_iterator<char>(input)), {});
    EXPECT_FALSE(code.empty());

    neolint::DiagnosticsSink diagnostics;
    neolint::RuleConfig config;
    EXPECT_TRUE(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Action>(config, diagnostics), code,
        {"-std=c++20"}, fixture, NEOLINT_TEST_CLANG_TOOL));
    return diagnostics.Diagnostics();
}
}

TEST(PlatformUtilityDuplicateRuleTests, ReportsKnownUtilityAliases)
{
    const auto diagnostics = AnalyzeFixture("violates.cpp");

    ASSERT_EQ(diagnostics.size(), 2u);
    EXPECT_EQ(diagnostics[0].ruleId, "neowpf-platform-utility-duplicate");
    EXPECT_EQ(diagnostics[1].ruleId, "neowpf-platform-utility-duplicate");
}

TEST(PlatformUtilityDuplicateRuleTests, IgnoresPlatformDefinitionsAndDomainFunctions)
{
    EXPECT_TRUE(AnalyzeFixture("compliant.cpp").empty());
}
