#include <NeoLint/Driver/Analyzer.h>
#include <NeoLint/Driver/AnalysisActions.h>
#include <NeoLint/Driver/AnalysisCache.h>
#include <NeoLint/Core/RuleRegistry.h>
#include <NeoLint/Reporting/JsonReportWriter.h>
#include <NeoLint/Diagnostics/Logger.h>

#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <algorithm>
#include <string>
#include <utility>

namespace neolint
{
namespace
{
constexpr std::size_t kCacheSaveInterval = 50;

class SingleCommandDatabase final : public clang::tooling::CompilationDatabase
{
public:
    explicit SingleCommandDatabase(clang::tooling::CompileCommand command)
        : command_(std::move(command)) {}

    std::vector<clang::tooling::CompileCommand> getCompileCommands(
        llvm::StringRef) const override
    {
        return {command_};
    }

    std::vector<std::string> getAllFiles() const override
    {
        return {command_.Filename};
    }

private:
    clang::tooling::CompileCommand command_;
};

const clang::tooling::CompileCommand& SelectCommand(
    const std::vector<clang::tooling::CompileCommand>& commands,
    const std::string& preferredConfig)
{
    if (!preferredConfig.empty())
    {
        const std::string marker = "CMAKE_INTDIR=\"" + preferredConfig + "\"";
        for (const auto& command : commands)
        {
            if (std::any_of(command.CommandLine.begin(), command.CommandLine.end(),
                [&](const std::string& argument) {
                    return argument.find(marker) != std::string::npos;
                }))
            {
                return command;
            }
        }
    }
    return commands.front();
}

bool IsUnder(const std::filesystem::path& file, const std::filesystem::path& root)
{
    const auto relative = file.lexically_relative(root);
    return !relative.empty() && relative != "." &&
        relative.native().find(L"..") != 0 && !relative.is_absolute();
}

std::vector<std::filesystem::path> NormalizeRoots(
    const std::vector<std::filesystem::path>& roots)
{
    std::vector<std::filesystem::path> normalized;
    for (const auto& root : roots) normalized.push_back(driver::AnalysisCache::NormalizePath(root));
    return normalized;
}

std::vector<std::string> SelectFiles(
    const clang::tooling::CompilationDatabase& database,
    const std::vector<std::filesystem::path>& roots)
{
    std::vector<std::string> files;
    for (const auto& file : database.getAllFiles())
    {
        const auto normalized = driver::AnalysisCache::NormalizePath(file);
        if (roots.empty() || std::any_of(roots.begin(), roots.end(),
            [&](const auto& root) { return IsUnder(normalized, root); }))
        {
            files.push_back(file);
        }
    }
    return files;
}
}

int Analyzer::Run(const AnalysisOptions& options, DiagnosticsSink& diagnostics) const
{
    diagnostics::Logger::Info("starting analysis");
    diagnostics::Logger::Info("compile database: " + options.compilationDatabase.string());
    diagnostics::Logger::Info("cache: " + (options.cache.empty() ? "disabled" : options.cache.string()));
    std::string error;
    const auto config = RuleConfig::Load(options.config, error);
    if (!error.empty())
    {
        diagnostics::Logger::Error(error);
        return 2;
    }

    for (const auto& configuredRule : config.RuleIds())
    {
        if (!RuleRegistry::Instance().Create(configuredRule))
        {
            diagnostics::Logger::Error("unknown rule in config: " + configuredRule);
            return 2;
        }
    }

    auto database = clang::tooling::JSONCompilationDatabase::loadFromFile(
        options.compilationDatabase.string(), error,
        clang::tooling::JSONCommandLineSyntax::AutoDetect);
    if (!database)
    {
        diagnostics::Logger::Error("cannot load compilation database: " + error);
        return 2;
    }

    const auto sourceRoots = NormalizeRoots(options.sourceRoots);
    diagnostics::Logger::Info("source roots: " + std::to_string(sourceRoots.size()));
    const auto files = SelectFiles(*database, sourceRoots);
    if (files.empty())
    {
        diagnostics::Logger::Error(sourceRoots.empty() ?
            "compilation database contains no files" :
            "compilation database contains no files under the configured source roots");
        return 2;
    }
    diagnostics::Logger::Info("selected " + std::to_string(files.size()) + " source files");

    auto cache = driver::AnalysisCache::Load(options.cache, options.config, sourceRoots);
    diagnostics::Logger::Info("loaded " + std::to_string(cache.EntryCount()) +
                              " reusable cache entries");
    int result = 0;
    for (std::size_t index = 0; index < files.size(); ++index)
    {
        const auto& file = files[index];
        const auto commands = database->getCompileCommands(file);
        if (commands.empty())
        {
            diagnostics::Logger::Warning("no compile command for " + file + ", skipping");
            continue;
        }
        const auto& selectedCommand = SelectCommand(commands, options.buildConfig);
        const auto sourceStamp = driver::AnalysisCache::FileStamp(file);
        const auto commandSignature = driver::AnalysisCache::CommandSignature({selectedCommand});
        DiagnosticList fileDiagnostics;
        if (cache.TryGet(file, sourceStamp, commandSignature, fileDiagnostics))
        {
            diagnostics::Logger::Info("[" + std::to_string(index + 1) + "/" +
                                      std::to_string(files.size()) + "] cached " + file);
            for (auto& diagnostic : fileDiagnostics) diagnostics.Report(std::move(diagnostic));
            continue;
        }

        diagnostics::Logger::Info("[" + std::to_string(index + 1) + "/" +
                                  std::to_string(files.size()) + "] analyzing " + file);
        DiagnosticsSink analyzedFile;
        SingleCommandDatabase singleDatabase(selectedCommand);
        clang::tooling::ClangTool tool(singleDatabase, {file});
        driver::ActionFactory factory(config, analyzedFile, options.strict);
        result = std::max(result, tool.run(&factory));
        fileDiagnostics = analyzedFile.Diagnostics();
        for (const auto& diagnostic : fileDiagnostics) diagnostics.Report(diagnostic);
        cache.Store(file, sourceStamp, commandSignature, fileDiagnostics);
        if (!options.cache.empty() &&
            ((index + 1) % kCacheSaveInterval == 0 || index + 1 == files.size()) &&
            !cache.Save(error))
        {
            diagnostics::Logger::Error(error);
            return 2;
        }
        diagnostics::Logger::Info("completed " + file + " (" +
                                  std::to_string(fileDiagnostics.size()) + " diagnostics)");
    }

    diagnostics.WriteHuman("NeoLint");
    if (!options.report.empty() &&
        !JsonReportWriter::Write(options.report, diagnostics.Diagnostics(), error))
    {
        diagnostics::Logger::Error(error);
        return 2;
    }
    if (!options.cache.empty() && !cache.Save(error))
    {
        diagnostics::Logger::Error(error);
        return 2;
    }
    diagnostics::Logger::Info("analysis complete");
    return result == 0 && !diagnostics.HasErrors() ? 0 : 1;
}
}
