#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleConfig.h>
#include <NeoLint/Rules/DPRegistrationLocationRule.h>
#include <NeoLint/Rules/PropertySourceTagRule.h>

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
    MatchAdapter(neolint::PropertySourceTagRule& rule, neolint::RuleContext& context)
        : rule_(rule), context_(context) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        rule_.OnMatch(result, context_);
    }

private:
    neolint::PropertySourceTagRule& rule_;
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
    neolint::PropertySourceTagRule rule_;
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

std::string ReadFixture(const char* filename)
{
    const auto path = std::filesystem::path(NEOLINT_TEST_SOURCE_DIR) /
        "tests/fixtures/property_source_tag" / filename;
    std::ifstream input(path);
    return {std::istreambuf_iterator<char>(input), {}};
}

neolint::DiagnosticList AnalyzeFixture(const char* filename,
                                       const neolint::RuleConfig& config = {})
{
    neolint::DiagnosticsSink diagnostics;
    const auto code = ReadFixture(filename);
    EXPECT_FALSE(code.empty());
    EXPECT_EQ(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Action>(config, diagnostics), code, {"-std=c++20"}, filename), true);
    return diagnostics.Diagnostics();
}

class DPMatchAdapter final : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    DPMatchAdapter(neolint::DPRegistrationLocationRule& rule,
                   neolint::RuleContext& context)
        : rule_(rule), context_(context) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        rule_.OnMatch(result, context_);
    }

private:
    neolint::DPRegistrationLocationRule& rule_;
    neolint::RuleContext& context_;
};

class DPConsumer final : public clang::ASTConsumer
{
public:
    DPConsumer(clang::ASTContext& ast, const neolint::RuleConfig& config,
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
    neolint::DPRegistrationLocationRule rule_;
    neolint::RuleContext context_;
    DPMatchAdapter adapter_;
    clang::ast_matchers::MatchFinder finder_;
};

class DPAction final : public clang::ASTFrontendAction
{
public:
    DPAction(const neolint::RuleConfig& config, neolint::DiagnosticsSink& diagnostics)
        : config_(config), diagnostics_(diagnostics) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler, llvm::StringRef) override
    {
        return std::make_unique<DPConsumer>(compiler.getASTContext(), config_, diagnostics_);
    }

private:
    const neolint::RuleConfig& config_;
    neolint::DiagnosticsSink& diagnostics_;
};

    neolint::DiagnosticList AnalyzeDPFixture(const char* fixture, const char* sourceName,
                         const neolint::RuleConfig& config = {})
{
    const auto path = std::filesystem::path(NEOLINT_TEST_SOURCE_DIR) /
        "tests/fixtures/dp_registration_location" / fixture;
    std::ifstream input(path);
    const std::string code((std::istreambuf_iterator<char>(input)), {});
    EXPECT_FALSE(code.empty());

    neolint::DiagnosticsSink diagnostics;
    EXPECT_EQ(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<DPAction>(config, diagnostics), code,
        {"-std=c++20"}, sourceName), true);
    return diagnostics.Diagnostics();
}
}

TEST(PropertySourceTagRuleTests, ReportsLocalSourceInTriggerWrapper)
{
    const auto diagnostics = AnalyzeFixture("violates_wrapper.cpp");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].ruleId, "neowpf-property-source-tag");
    EXPECT_EQ(diagnostics[0].severity, neolint::RuleSeverity::Warning);
}

TEST(PropertySourceTagRuleTests, AcceptsTriggerSourceInTriggerWrapper)
{
    EXPECT_TRUE(AnalyzeFixture("compliant_wrapper.cpp").empty());
}

TEST(PropertySourceTagRuleTests, HonorsPathScopedDisable)
{
    const auto path = std::filesystem::temp_directory_path() /
        "neolint-rule-path-override-test.json";
    std::ofstream(path) << R"({"rules":{"neowpf-property-source-tag":{"paths":{"*violates_wrapper.cpp":{"enabled":false}}}}})";

    std::string error;
    const auto config = neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);

    ASSERT_TRUE(error.empty()) << error;
    EXPECT_TRUE(AnalyzeFixture("violates_wrapper.cpp", config).empty());
}

TEST(PropertySourceTagRuleTests, HonorsPathScopedSeverity)
{
    const auto path = std::filesystem::temp_directory_path() /
        "neolint-rule-path-severity-test.json";
    std::ofstream(path) << R"({"rules":{"neowpf-property-source-tag":{"paths":{"*violates_wrapper.cpp":{"severity":"error"}}}}})";

    std::string error;
    const auto config = neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);

    ASSERT_TRUE(error.empty()) << error;
    const auto diagnostics = AnalyzeFixture("violates_wrapper.cpp", config);
    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].severity, neolint::RuleSeverity::Error);
}

TEST(PropertySourceTagRuleTests, HonorsConfiguredWrapperMethodNames)
{
    const auto path = std::filesystem::temp_directory_path() /
        "neolint-wrapper-options-test.json";
    std::ofstream(path) << R"({"rules":{"neowpf-property-source-tag":{"options":{"wrapper_method_names":["CustomWrapper"]}}}})";
    std::string error;
    const auto config = neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);
    ASSERT_TRUE(error.empty()) << error;
    EXPECT_TRUE(AnalyzeFixture("violates_wrapper.cpp", config).empty());
}

TEST(PropertySourceTagRuleTests, HonorsInlineSuppressionWithReason)
{
    EXPECT_TRUE(AnalyzeFixture("suppressed_wrapper.cpp").empty());
}

TEST(PropertySourceTagRuleTests, IgnoresInlineSuppressionForAnotherRule)
{
    const auto path = std::filesystem::path(NEOLINT_TEST_SOURCE_DIR) /
        "tests/fixtures/property_source_tag/suppressed_wrapper.cpp";
    std::ifstream input(path);
    std::string code((std::istreambuf_iterator<char>(input)), {});
    const auto marker = std::string("neowpf-property-source-tag");
    code.replace(code.find(marker), marker.size(), "different-rule");

    neolint::DiagnosticsSink diagnostics;
    neolint::RuleConfig config;
    EXPECT_EQ(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Action>(config, diagnostics), code,
        {"-std=c++20"}, "suppressed_wrapper.cpp"), true);
    ASSERT_EQ(diagnostics.Diagnostics().size(), 1u);
    EXPECT_EQ(diagnostics.Diagnostics()[0].ruleId, "neowpf-property-source-tag");
}

TEST(PropertySourceTagRuleTests, RequiresSuppressionReason)
{
    const auto path = std::filesystem::path(NEOLINT_TEST_SOURCE_DIR) /
        "tests/fixtures/property_source_tag/suppressed_wrapper.cpp";
    std::ifstream input(path);
    std::string code((std::istreambuf_iterator<char>(input)), {});
    const auto reason = std::string(": legacy source behavior");
    code.replace(code.find(reason), reason.size(), ":");

    neolint::DiagnosticsSink diagnostics;
    neolint::RuleConfig config;
    EXPECT_EQ(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Action>(config, diagnostics), code,
        {"-std=c++20"}, "suppressed_wrapper.cpp"), true);
    EXPECT_EQ(diagnostics.Diagnostics().size(), 1u);
}

TEST(DPRegistrationLocationRuleTests, AcceptsDedicatedPropertiesFile)
{
    EXPECT_TRUE(AnalyzeDPFixture("compliant.cpp", "ButtonProperties.cpp").empty());
}

TEST(DPRegistrationLocationRuleTests, ReportsRegistrationOutsidePropertiesFile)
{
    const auto diagnostics = AnalyzeDPFixture("violates.cpp", "Button.cpp");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].ruleId, "neowpf-dp-registration-location");
    EXPECT_EQ(diagnostics[0].severity, neolint::RuleSeverity::Warning);
}

TEST(DPRegistrationLocationRuleTests, IgnoresRegisterCallsOnBindingExpressions)
{
    const auto diagnostics = AnalyzeDPFixture("binding_expression.cpp", "BindingExpression.cpp");
    EXPECT_TRUE(diagnostics.empty());
}

TEST(DPRegistrationLocationRuleTests, HonorsConfiguredFileSuffixes)
{
    const auto path = std::filesystem::temp_directory_path() /
        "neolint-dp-suffix-options-test.json";
    std::ofstream(path) << R"({"rules":{"neowpf-dp-registration-location":{"options":{"allowed_file_suffixes":["Special.cpp"]}}}})";
    std::string error;
    const auto config = neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);
    ASSERT_TRUE(error.empty()) << error;
    EXPECT_TRUE(AnalyzeDPFixture("violates.cpp", "ButtonSpecial.cpp", config).empty());
}

TEST(PropertySourceTagRuleTests, SupportsSameLineSuppression)
{
    const std::string code = R"(
enum class PropertyValueSource { Local, Trigger };
class UIElement
{
public:
    void SetNamedPropertyValue(int, int, PropertyValueSource);
    void SetTriggerPropertyValue(int property, int value)
    {
        SetNamedPropertyValue(property, value, PropertyValueSource::Local); // neolint-disable-line neowpf-property-source-tag: compatibility
    }
};
)";
    neolint::DiagnosticsSink diagnostics;
    neolint::RuleConfig config;
    ASSERT_TRUE(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Action>(config, diagnostics), code,
        {"-std=c++20"}, "same-line.cpp"));
    EXPECT_TRUE(diagnostics.Diagnostics().empty());
}

TEST(PropertySourceTagRuleTests, SupportsFileLevelSuppressionOnFirstLine)
{
    const std::string code = R"(// neolint-disable-file neowpf-property-source-tag: compatibility
enum class PropertyValueSource { Local, Trigger };
class UIElement
{
public:
    void SetNamedPropertyValue(int, int, PropertyValueSource);
    void SetTriggerPropertyValue(int property, int value)
    {
        SetNamedPropertyValue(property, value, PropertyValueSource::Local);
    }
};
)";
    neolint::DiagnosticsSink diagnostics;
    neolint::RuleConfig config;
    ASSERT_TRUE(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Action>(config, diagnostics), code,
        {"-std=c++20"}, "file-level.cpp"));
    EXPECT_TRUE(diagnostics.Diagnostics().empty());
}
