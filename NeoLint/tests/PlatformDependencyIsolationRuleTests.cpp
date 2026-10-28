#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleConfig.h>
#include <NeoLint/Rules/PlatformDependencyIsolationRule.h>

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
    MatchAdapter(neolint::PlatformDependencyIsolationRule& rule,
                 neolint::RuleContext& context)
        : rule_(rule), context_(context) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        rule_.OnMatch(result, context_);
    }

private:
    neolint::PlatformDependencyIsolationRule& rule_;
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
    neolint::PlatformDependencyIsolationRule rule_;
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
        auto& sourceManager = compiler.getSourceManager();
        const auto mainFile = sourceManager.getLocForStartOfFile(sourceManager.getMainFileID());
        const auto filename = sourceManager.getFilename(mainFile).str();
        const auto settings = config_.SettingsFor(
            "neowpf-platform-dependency-isolation", filename);
        if (settings.enabled)
        {
            compiler.getPreprocessor().addPPCallbacks(
                neolint::PlatformDependencyIsolationRule::CreatePreprocessorCallbacks(
                    settings.options, config_, diagnostics_, sourceManager, false));
        }
        return std::make_unique<Consumer>(compiler.getASTContext(), config_, diagnostics_);
    }

private:
    const neolint::RuleConfig& config_;
    neolint::DiagnosticsSink& diagnostics_;
};

std::filesystem::path FixtureDirectory()
{
    return std::filesystem::path(NEOLINT_TEST_SOURCE_DIR) /
        "tests/fixtures/platform_dependency_isolation";
}

neolint::RuleConfig LoadConfig(const std::string& options)
{
    const auto path = std::filesystem::temp_directory_path() /
        "neolint-platform-dependency-isolation.json";
    {
        std::ofstream output(path);
        output << "{\"rules\":{\"neowpf-platform-dependency-isolation\":{"
                  "\"enabled\":true,\"severity\":\"error\",\"options\":"
               << options << "}}}";
    }
    std::string error;
    auto config = neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);
    EXPECT_TRUE(error.empty()) << error;
    return config;
}

neolint::DiagnosticList AnalyzeFixture(const char* filename,
                                       const neolint::RuleConfig& config)
{
    const auto path = FixtureDirectory() / filename;
    std::ifstream input(path);
    const std::string code((std::istreambuf_iterator<char>(input)), {});
    EXPECT_FALSE(code.empty());

    neolint::DiagnosticsSink diagnostics;
    EXPECT_TRUE(clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Action>(config, diagnostics), code, {"-std=c++20"},
        path.string(), NEOLINT_TEST_CLANG_TOOL));
    return diagnostics.Diagnostics();
}
}

TEST(PlatformDependencyIsolationRuleTests, ReportsTransitiveHeaderAndAstLeaks)
{
    const auto config = LoadConfig(
        R"({"platforms":["windows","linux","android"]})");
    const auto diagnostics = AnalyzeFixture("violates.cpp", config);

    ASSERT_GE(diagnostics.size(), 2u);
    EXPECT_EQ(diagnostics[0].ruleId, "neowpf-platform-dependency-isolation");
    EXPECT_NE(diagnostics[0].message.find("include chain"), std::string::npos);
    EXPECT_NE(diagnostics[1].message.find("declaration unavailable"), std::string::npos);
}

TEST(PlatformDependencyIsolationRuleTests, HonorsCommentSuppressionForAstUse)
{
    const auto config = LoadConfig(
        R"({"platforms":["windows","linux","android"]})");
    const auto diagnostics = AnalyzeFixture("suppressed.cpp", config);

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_NE(diagnostics[0].message.find("include chain"), std::string::npos);
}

TEST(PlatformDependencyIsolationRuleTests, AllowsConfiguredLinuxAndAndroidSharing)
{
    const auto config = LoadConfig(
        R"({"platforms":["windows","linux","android"],"path_platforms":{"*posix.cpp":"linux,android"},"header_platforms":{"*Posix.h":"linux,android"}})");
    EXPECT_TRUE(AnalyzeFixture("posix.cpp", config).empty());
}

TEST(PlatformDependencyIsolationRuleTests, AllowsPlatformSpecificImplementation)
{
    const auto config = LoadConfig(
        R"({"platforms":["windows","linux","android"],"path_platforms":{"*windows_platform.cpp":"windows"}})");
    EXPECT_TRUE(AnalyzeFixture("windows_platform.cpp", config).empty());
}

TEST(PlatformDependencyIsolationRuleTests, DoesNotTreatVcpkgTripletNamesAsSystemDirectories)
{
    const auto config = LoadConfig(
        R"({"platforms":["windows","linux","android"]})");
    const auto tempRoot = std::filesystem::temp_directory_path() /
        "neolint-platform-dependency-triplet-path";
    const auto code = "#include <dependency.h>\nint main() { return 0; }\n";

    for (const auto* triplet : {"x64-linux", "x64-android"})
    {
        const auto tripletRoot = tempRoot / triplet;
        const auto includeDirectory = tripletRoot / "include";
        std::filesystem::remove_all(tripletRoot);
        std::filesystem::create_directories(includeDirectory);
        {
            std::ofstream header(includeDirectory / "dependency.h");
            header << "#pragma once\n";
        }

        neolint::DiagnosticsSink diagnostics;
        ASSERT_TRUE(clang::tooling::runToolOnCodeWithArgs(
            std::make_unique<Action>(config, diagnostics), code,
            {"-std=c++20", "-I" + includeDirectory.string()},
            (tripletRoot / "input.cpp").string(), NEOLINT_TEST_CLANG_TOOL));
        EXPECT_TRUE(diagnostics.Diagnostics().empty()) << triplet;
        std::filesystem::remove_all(tripletRoot);
    }
}