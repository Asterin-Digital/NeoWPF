#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleConfig.h>
#include <NeoLint/Core/RuleDiagnostic.h>
#include <NeoLint/Driver/AnalysisCache.h>
#include <NeoLint/Driver/Analyzer.h>
#include <NeoLint/Rules/SourceLayoutRule.h>
#include <NeoLint/Reporting/JsonReportWriter.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <algorithm>
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

TEST(RuleConfigTests, PreservesPathOverrideOrder)
{
    const auto path = TemporaryPath("neolint-ordered-path-config-test.json");
    std::ofstream(path) << R"({"rules":{"example-rule":{"paths":{"src/Strict/*":{"severity":"error"},"src/*":{"severity":"info"}}}}})";

    std::string error;
    const auto config = neolint::RuleConfig::Load(path, error);
    std::filesystem::remove(path);

    ASSERT_TRUE(error.empty()) << error;
    EXPECT_EQ(config.SettingsFor("example-rule", "src/Strict/File.cpp").severity,
              neolint::RuleSeverity::Error);
}

TEST(RuleConfigTests, RejectsUnknownSettingsAndInvalidOptionTypes)
{
    const auto unknownPath = TemporaryPath("neolint-unknown-setting-test.json");
    std::ofstream(unknownPath) << R"({"rules":{"example-rule":{"severty":"error"}}})";
    std::string error;
    neolint::RuleConfig::Load(unknownPath, error);
    std::filesystem::remove(unknownPath);
    EXPECT_FALSE(error.empty());

    const auto optionsPath = TemporaryPath("neolint-invalid-option-test.json");
    std::ofstream(optionsPath) << R"({"rules":{"example-rule":{"options":{"names":["valid",3]}}}})";
    error.clear();
    neolint::RuleConfig::Load(optionsPath, error);
    std::filesystem::remove(optionsPath);
    EXPECT_FALSE(error.empty());
}

TEST(SourceLayoutRuleTests, UsesEachSourceRootsOwnName)
{
    const auto testRoot = TemporaryPath("neolint-layout-test-src-parent");
    const auto sourceRoot = testRoot / "src";
    const auto includeRoot = testRoot / "include";
    const auto unrelatedRoot = testRoot / "Other";
    std::filesystem::remove_all(testRoot);
    std::filesystem::create_directories(sourceRoot);
    std::filesystem::create_directories(includeRoot);
    std::filesystem::create_directories(unrelatedRoot);
    std::ofstream(sourceRoot / "bad.h") << "";
    std::ofstream(includeRoot / "bad.cpp") << "";
    std::ofstream(includeRoot / "allowed.h") << "";
    std::ofstream(unrelatedRoot / "allowed.h") << "";

    neolint::DiagnosticsSink diagnostics;
    neolint::SourceLayoutRule{}.Run({sourceRoot, includeRoot}, {}, diagnostics);

    ASSERT_EQ(diagnostics.Diagnostics().size(), 2u);
    std::filesystem::remove_all(testRoot);
}

TEST(AnalysisCacheTests, InvalidatesEntriesWhenADependencyChanges)
{
    const auto root = TemporaryPath("neolint-cache-v2-test");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    const auto source = root / "main.cpp";
    const auto dependency = root / "shared.h";
    const auto config = root / "config.json";
    const auto executable = root / "NeoLint.exe";
    const auto cachePath = root / "cache.json";
    std::ofstream(source) << "int main() {}\n";
    std::ofstream(dependency) << "struct Shared {};\n";
    std::ofstream(config) << "{\"rules\":{}}";
    std::ofstream(executable) << "tool";

    auto cache = neolint::driver::AnalysisCache::Load(cachePath, config, executable);
    const neolint::DiagnosticList expected;
    cache.Store(source, neolint::driver::AnalysisCache::FileStamp(source), "command",
                {source, dependency}, expected);
    std::string error;
    ASSERT_TRUE(cache.Save(error)) << error;

    auto loaded = neolint::driver::AnalysisCache::Load(cachePath, config, executable);
    neolint::DiagnosticList actual;
    ASSERT_TRUE(loaded.TryGet(source, neolint::driver::AnalysisCache::FileStamp(source),
                              "command", actual));
    std::ofstream(dependency) << "struct Shared { int value; };\n";
    auto nextRun = neolint::driver::AnalysisCache::Load(cachePath, config, executable);
    ASSERT_FALSE(nextRun.TryGet(source, neolint::driver::AnalysisCache::FileStamp(source),
                                "command", actual));
    std::filesystem::remove_all(root);
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

TEST(DiagnosticsSinkTests, SortsAndDeduplicatesDiagnostics)
{
    neolint::DiagnosticsSink sink;
    sink.Report({"rule-b", neolint::RuleSeverity::Warning, {"z.cpp", 2, 1}, "later"});
    sink.Report({"rule-a", neolint::RuleSeverity::Error, {"a.cpp", 4, 2}, "first"});
    sink.Report({"rule-a", neolint::RuleSeverity::Error, {"a.cpp", 4, 2}, "first"});

    sink.SortAndDeduplicate();

    ASSERT_EQ(sink.Diagnostics().size(), 2u);
    EXPECT_EQ(sink.Diagnostics()[0].location.file, "a.cpp");
    EXPECT_EQ(sink.Diagnostics()[1].location.file, "z.cpp");
}

TEST(AnalyzerTests, SerialAndParallelRunsProduceIdenticalReports)
{
    const auto originalWorkingDirectory = std::filesystem::current_path();
    const auto root = TemporaryPath("neolint-analyzer-parallel-test");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    const auto sharedHeader = root / "shared.h";
    std::ofstream(sharedHeader) << "inline int Trim() { return 1; }\n";

    nlohmann::json commands = nlohmann::json::array();
    for (int index = 0; index < 4; ++index)
    {
        const auto source = root / ("source" + std::to_string(index) + ".cpp");
        std::ofstream output(source);
        if (index == 0) output << "#include \"shared.h\"\n";
        output << "namespace sample" << index
               << " { int Value() { return " << index << "; } }\n";
        commands.push_back({
            {"directory", root.string()},
            {"file", source.string()},
            {"arguments", {"clang++", "-std=c++20", "-fsyntax-only", source.string()}},
        });
    }
    std::ofstream(root / "compile_commands.json") << commands.dump();
    std::ofstream(root / "config.json") <<
        R"({"rules":{"neowpf-platform-utility-duplicate":{"enabled":true}}})";

    neolint::AnalysisOptions options;
    options.compilationDatabase = root / "compile_commands.json";
    options.config = root / "config.json";
    options.sourceRoots = {root};
    options.cache = root / "serial-cache.json";
    options.report = root / "serial.json";
    options.jobs = 1;
    neolint::DiagnosticsSink serialDiagnostics;
    const auto serialStatus = neolint::Analyzer{}.Run(options, serialDiagnostics);
    ASSERT_EQ(serialStatus, 0);
    ASSERT_EQ(serialDiagnostics.Diagnostics().size(), 1u);

    std::ifstream cacheInput(root / "serial-cache.json");
    const auto cacheDocument = nlohmann::json::parse(cacheInput);
    const auto sourceKey = neolint::driver::AnalysisCache::NormalizePath(
        root / "source0.cpp").generic_string();
    const auto dependencies = cacheDocument["files"][sourceKey]["dependencies"];
    EXPECT_TRUE(std::any_of(dependencies.begin(), dependencies.end(), [&](const auto& dependency) {
        return dependency["path"] == neolint::driver::AnalysisCache::NormalizePath(
            sharedHeader).generic_string();
    }));
    cacheInput.close();

    options.report = root / "parallel.json";
    options.cache = root / "parallel-cache.json";
    options.jobs = 4;
    neolint::DiagnosticsSink parallelDiagnostics;
    const auto parallelStatus = neolint::Analyzer{}.Run(options, parallelDiagnostics);

    ASSERT_EQ(parallelStatus, 0);
    std::ifstream serialInput(root / "serial.json");
    std::ifstream parallelInput(root / "parallel.json");
    const std::string serialReport{std::istreambuf_iterator<char>(serialInput), {}};
    const std::string parallelReport{std::istreambuf_iterator<char>(parallelInput), {}};
    EXPECT_EQ(serialReport, parallelReport);

    serialInput.close();
    parallelInput.close();

    options.cache = root / "serial-cache.json";
    options.report = root / "cached.json";
    options.jobs = 1;
    neolint::DiagnosticsSink cachedDiagnostics;
    ASSERT_EQ(neolint::Analyzer{}.Run(options, cachedDiagnostics), 0);
    std::ifstream cachedInput(root / "cached.json");
    const std::string cachedReport{std::istreambuf_iterator<char>(cachedInput), {}};
    EXPECT_EQ(serialReport, cachedReport);
    cachedInput.close();

    std::ofstream(sharedHeader) << "inline int Other() { return 1; }\n";
    options.report = root / "updated.json";
    neolint::DiagnosticsSink updatedDiagnostics;
    ASSERT_EQ(neolint::Analyzer{}.Run(options, updatedDiagnostics), 0);
    EXPECT_TRUE(updatedDiagnostics.Diagnostics().empty());

    EXPECT_EQ(std::filesystem::current_path(), originalWorkingDirectory);
    std::filesystem::remove_all(root);
}
