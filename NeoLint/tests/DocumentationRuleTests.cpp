#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleConfig.h>
#include <NeoLint/Rules/DocumentationRule.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

namespace
{
class MatchAdapter final : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    MatchAdapter(neolint::DocumentationRule& rule, neolint::RuleContext& context)
        : rule_(rule), context_(context) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        rule_.OnMatch(result, context_);
    }

private:
    neolint::DocumentationRule& rule_;
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
        rule_.RegisterMatchers(finder_, adapter_);
    }

    void HandleTranslationUnit(clang::ASTContext&) override
    {
        finder_.matchAST(context_.AST());
    }

private:
    neolint::DocumentationRule rule_;
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

neolint::DiagnosticList AnalyzeSetGet(const std::string& code)
{
    neolint::DiagnosticsSink diagnostics;
    neolint::RuleConfig config;
    EXPECT_TRUE(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Action>(config, diagnostics), code,
        {"-std=c++20"}, "documentation.h"));
    return diagnostics.Diagnostics();
}

std::filesystem::path CreateTempDirectory()
{
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto path = std::filesystem::temp_directory_path() /
        ("neolint-documentation-" + std::to_string(timestamp));
    std::filesystem::create_directories(path);
    return path;
}
}

TEST(DocumentationRuleTests, WarnsWhenPairedMethodIsUndocumented)
{
    const auto diagnostics = AnalyzeSetGet(R"(
class Value {
public:
    /*
        @summary
        Sets the value.
    */
    void SetValue(int value);
    int GetValue() const;
};
)");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].ruleId, "neowpf-documentation");
    EXPECT_EQ(diagnostics[0].severity, neolint::RuleSeverity::Warning);
    EXPECT_NE(diagnostics[0].message.find("GetValue"), std::string::npos);
}

TEST(DocumentationRuleTests, DoesNotPairMethodsAcrossClasses)
{
    EXPECT_TRUE(AnalyzeSetGet(R"(
class Writer {
public:
    /** @summary Sets the value. */
    void SetValue(int value);
};
class Reader {
public:
    int GetValue() const;
};
)").empty());
}

TEST(DocumentationRuleTests, ReportsNoteAndTodoInHeadersButIgnoresSourceFiles)
{
    const auto directory = CreateTempDirectory();
    const auto header = directory / "sample.h";
    const auto otherHeader = directory / "sample.hpp";
    const auto suppressedHeader = directory / "suppressed.h";
    const auto source = directory / "sample.cpp";
    {
        std::ofstream output(header);
        output << "// @note temporary\n// @todo remove this\n";
    }
    {
        std::ofstream output(otherHeader);
        output << "// @note other header extensions are not checked\n";
    }
    {
        std::ofstream output(suppressedHeader);
        output << "// neolint-disable-next-line neowpf-documentation: tracked separately\n"
                  "// @todo intentionally suppressed\n";
    }
    {
        std::ofstream output(source);
        output << "// @todo source files are not checked\n";
    }

    neolint::DocumentationRule rule;
    neolint::RuleConfig config;
    neolint::DiagnosticsSink diagnostics;
    rule.Run({directory}, config, diagnostics);
    std::filesystem::remove_all(directory);

    ASSERT_EQ(diagnostics.Diagnostics().size(), 2u);
    EXPECT_EQ(diagnostics.Diagnostics()[0].ruleId, "neowpf-documentation");
    EXPECT_EQ(diagnostics.Diagnostics()[0].severity, neolint::RuleSeverity::Warning);
    EXPECT_EQ(diagnostics.Diagnostics()[1].severity, neolint::RuleSeverity::Warning);
}