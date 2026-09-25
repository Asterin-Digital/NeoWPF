#include <NeoLint/Core/RuleConfig.h>
#include <NeoLint/Core/RuleDiagnostic.h>
#include <NeoLint/Reporting/JsonReportWriter.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace
{
std::filesystem::path TemporaryPath(const char* name)
{
    return std::filesystem::temp_directory_path() / name;
}
}

TEST(RuleConfigTests, LoadsRuleSettingsFromJson)
{
    const auto path = TemporaryPath("neolint-config-test.json");
    std::ofstream(path) << R"({"rules":{"example-rule":{"enabled":false,"severity":"error"}}})";

    std::string error;
    const auto config = neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);

    ASSERT_TRUE(error.empty()) << error;
    ASSERT_TRUE(config.HasRule("example-rule"));
    EXPECT_FALSE(config.SettingsFor("example-rule").enabled);
    EXPECT_EQ(config.SettingsFor("example-rule").severity, neolint::RuleSeverity::Error);
}

TEST(RuleConfigTests, RejectsInvalidSeverity)
{
    const auto path = TemporaryPath("neolint-invalid-config-test.json");
    std::ofstream(path) << R"({"rules":{"example-rule":{"severity":"loud"}}})";

    std::string error;
    neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);

    EXPECT_FALSE(error.empty());
}

TEST(RuleConfigTests, AppliesPathScopedOverrides)
{
    const auto path = TemporaryPath("neolint-path-config-test.json");
    std::ofstream(path) << R"({"rules":{"example-rule":{"severity":"warning","paths":{"src/Generated/*":{"enabled":false},"src/Strict/*.cpp":{"severity":"error"}}}}})";

    std::string error;
    const auto config = neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);

    ASSERT_TRUE(error.empty()) << error;
    EXPECT_FALSE(config.SettingsFor("example-rule", "src/Generated/File.cpp").enabled);
    EXPECT_EQ(config.SettingsFor("example-rule", "src/Strict/File.cpp").severity,
              neolint::RuleSeverity::Error);
    EXPECT_TRUE(config.SettingsFor("example-rule", "src/Other/File.cpp").enabled);
}

TEST(RuleConfigTests, RejectsInvalidPathOverrides)
{
    const auto path = TemporaryPath("neolint-invalid-path-config-test.json");
    std::ofstream(path) << R"({"rules":{"example-rule":{"paths":[]}}})";

    std::string error;
    neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);

    EXPECT_FALSE(error.empty());
}

TEST(JsonReportWriterTests, WritesStructuredDiagnostics)
{
    const auto path = TemporaryPath("neolint-report-test.json");
    neolint::DiagnosticList diagnostics{{
        "example-rule",
        neolint::RuleSeverity::Warning,
        {"fixture.cpp", 4, 7},
        "message with \"quotes\"",
    }};

    std::string error;
    ASSERT_TRUE(neolint::JsonReportWriter::Write(path, diagnostics, error)) << error;
    std::string content;
    {
        std::ifstream input(path);
        content.assign((std::istreambuf_iterator<char>(input)), {});
    }
    std::filesystem::remove(path);

    const auto report = nlohmann::json::parse(content);
    ASSERT_EQ(report.size(), 1u);
    EXPECT_EQ(report[0]["rule"], "example-rule");
    EXPECT_EQ(report[0]["message"], "message with \"quotes\"");
}
