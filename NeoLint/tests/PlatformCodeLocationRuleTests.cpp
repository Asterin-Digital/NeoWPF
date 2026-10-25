#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleConfig.h>
#include <NeoLint/Core/RuleRegistry.h>
#include <NeoLint/Rules/PlatformCodeLocationRule.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

namespace
{
class MatchAdapter final : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    MatchAdapter(neolint::PlatformCodeLocationRule& rule, neolint::RuleContext& context)
        : rule_(rule), context_(context) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        rule_.OnMatch(result, context_);
    }

private:
    neolint::PlatformCodeLocationRule& rule_;
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
    neolint::PlatformCodeLocationRule rule_;
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

neolint::RuleConfig LoadConfig(const std::string& options)
{
    const auto path = std::filesystem::temp_directory_path() /
        "neolint-platform-code-location.json";
    {
        std::ofstream output(path);
        output << "{\"rules\":{\"neowpf-platform-code-location\":{"
                  "\"enabled\":true,\"severity\":\"error\",\"options\":"
               << options << "}}}";
    }
    std::string error;
    auto config = neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);
    EXPECT_TRUE(error.empty()) << error;
    return config;
}

neolint::DiagnosticList AnalyzeAst(const std::string& code, const char* filename,
                                   const neolint::RuleConfig& config)
{
    neolint::DiagnosticsSink diagnostics;
    EXPECT_TRUE(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Action>(config, diagnostics), code, {"-std=c++20"}, filename));
    return diagnostics.Diagnostics();
}

std::filesystem::path FixtureDirectory()
{
    return std::filesystem::path(NEOLINT_TEST_SOURCE_DIR) /
        "tests/fixtures/platform_code_location";
}
}

TEST(PlatformCodeLocationRuleTests, RegistersAsBothAstAndTreeRuleWithOneId)
{
    const auto& registry = neolint::RuleRegistry::Instance();
    EXPECT_NE(registry.Create("neowpf-platform-code-location"), nullptr);
    EXPECT_NE(registry.CreateTree("neowpf-platform-code-location"), nullptr);
    const auto ids = registry.Ids();
    EXPECT_EQ(std::count(ids.begin(), ids.end(), "neowpf-platform-code-location"), 1);
}

TEST(PlatformCodeLocationRuleTests, ReportsConfiguredPortableDefinitionsOutsidePlatform)
{
    const auto config = LoadConfig(
        R"({"platform_symbols":{"PortableWindowsImplementation":"windows"}})");
    const auto diagnostics = AnalyzeAst(
        "int PortableWindowsImplementation() { return 1; }", "shared.cpp", config);

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].ruleId, "neowpf-platform-code-location");
    EXPECT_NE(diagnostics[0].message.find("Platform/Windows/"), std::string::npos);
}

TEST(PlatformCodeLocationRuleTests, AcceptsConfiguredPortableDefinitionsInPlatformFolder)
{
    const auto config = LoadConfig(
        R"({"platform_symbols":{"PortableWindowsImplementation":"windows"}})");
    EXPECT_TRUE(AnalyzeAst("int PortableWindowsImplementation() { return 1; }",
                           "Platform/Windows/shared.cpp", config).empty());
}

TEST(PlatformCodeLocationRuleTests, ChecksGuardedAndConfiguredFilesAndHonorsComment)
{
    const auto config = LoadConfig(
        R"({"platform_paths":{"*portable_windows.cpp":"windows"}})");
    neolint::PlatformCodeLocationRule rule;
    neolint::DiagnosticsSink diagnostics;
    rule.Run({FixtureDirectory()}, config, diagnostics);

    ASSERT_EQ(diagnostics.Diagnostics().size(), 2u);
    EXPECT_EQ(diagnostics.Diagnostics()[0].ruleId, "neowpf-platform-code-location");
    EXPECT_EQ(diagnostics.Diagnostics()[1].ruleId, "neowpf-platform-code-location");
}

TEST(PlatformCodeLocationRuleTests, AllowsDefinitionsGuardedForMultiplePlatforms)
{
    const auto config = LoadConfig("{}");
    neolint::PlatformCodeLocationRule rule;
    neolint::DiagnosticsSink diagnostics;
    rule.Run({FixtureDirectory()}, config, diagnostics);

    EXPECT_EQ(diagnostics.Diagnostics().size(), 1u);
}