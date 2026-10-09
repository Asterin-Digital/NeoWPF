#include <NeoLint/Driver/Analyzer.h>
#include <NeoLint/Driver/AnalysisActions.h>
#include <NeoLint/Driver/AnalysisCache.h>
#include <NeoLint/Core/RuleRegistry.h>
#include <NeoLint/Reporting/JsonReportWriter.h>
#include <NeoLint/Diagnostics/Logger.h>

#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/VirtualFileSystem.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
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
    *relative.begin() != ".." && !relative.is_absolute();
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

struct AnalysisJob
{
    std::size_t fileIndex;
    std::string file;
    clang::tooling::CompileCommand command;
    std::string sourceStamp;
    std::string commandSignature;
};

struct FileResult
{
    DiagnosticList diagnostics;
    int status = 0;
};

}

int Analyzer::Run(const AnalysisOptions& options, DiagnosticsSink& diagnostics) const
{
    diagnostics::Logger::Info("starting analysis");
    diagnostics::Logger::Info("compile database: " + options.compilationDatabase.string());
    diagnostics::Logger::Info("cache: " + (options.cache.empty() ? "disabled" : options.cache.string()));
    const auto cachePath = options.cache.empty() ? std::filesystem::path{} :
        driver::AnalysisCache::NormalizePath(options.cache);
    const auto reportPath = options.report.empty() ? std::filesystem::path{} :
        driver::AnalysisCache::NormalizePath(options.report);
    std::string error;
    const auto config = RuleConfig::Load(options.config, error);
    if (!error.empty())
    {
        diagnostics::Logger::Error(error);
        return 2;
    }

    for (const auto& configuredRule : config.RuleIds())
    {
        auto astRule = RuleRegistry::Instance().Create(configuredRule);
        auto treeRule = RuleRegistry::Instance().CreateTree(configuredRule);
        if (!astRule && !treeRule)
        {
            diagnostics::Logger::Error("unknown rule in config: " + configuredRule);
            return 2;
        }
        for (const auto& settings : config.AllSettingsFor(configuredRule))
        {
            const bool valid = astRule ?
                astRule->ValidateOptions(settings.options, error) :
                treeRule->ValidateOptions(settings.options, error);
            if (!valid)
            {
                diagnostics::Logger::Error(error);
                return 2;
            }
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
    for (const auto& ruleId : RuleRegistry::Instance().Ids())
    {
        auto rule = RuleRegistry::Instance().CreateTree(ruleId);
        const auto settings = config.AllSettingsFor(ruleId);
        const bool enabled = std::any_of(settings.begin(), settings.end(),
            [](const RuleSettings& item) { return item.enabled; });
        if (rule && enabled)
        {
            rule->Run(sourceRoots, config, diagnostics);
        }
    }
    const auto files = SelectFiles(*database, sourceRoots);
    if (files.empty())
    {
        diagnostics::Logger::Error(sourceRoots.empty() ?
            "compilation database contains no files" :
            "compilation database contains no files under the configured source roots");
        return 2;
    }
    diagnostics::Logger::Info("selected " + std::to_string(files.size()) + " source files");

    auto cache = driver::AnalysisCache::Load(cachePath, options.config, options.executable);
    cache.Prune([&] {
        std::vector<std::filesystem::path> paths;
        paths.reserve(files.size());
        for (const auto& file : files) paths.emplace_back(file);
        return paths;
    }());
    diagnostics::Logger::Info("loaded " + std::to_string(cache.EntryCount()) +
                              " reusable cache entries");
    int result = 0;
    std::vector<FileResult> results(files.size());
    std::vector<AnalysisJob> jobs;
    jobs.reserve(files.size());
    std::size_t cacheHits = 0;
    std::atomic<std::size_t> completedFiles = 0;
    for (std::size_t index = 0; index < files.size(); ++index)
    {
        const auto& file = files[index];
        const auto commands = database->getCompileCommands(file);
        if (commands.empty())
        {
            diagnostics::Logger::Warning("no compile command for " + file + ", skipping");
            completedFiles.fetch_add(1, std::memory_order_relaxed);
            continue;
        }
        const auto& selectedCommand = SelectCommand(commands, options.buildConfig);
        const auto workingDirectory = driver::AnalysisCache::NormalizePath(selectedCommand.Directory);
        auto command = selectedCommand;
        command.Directory = workingDirectory.string();
        const auto sourceStamp = driver::AnalysisCache::FileStamp(file);
        const auto commandSignature = driver::AnalysisCache::CommandSignature({selectedCommand});
        if (cache.TryGet(file, sourceStamp, commandSignature, results[index].diagnostics))
        {
            const auto completed = completedFiles.fetch_add(1, std::memory_order_relaxed) + 1;
            ++cacheHits;
            diagnostics::Logger::Info("[" + std::to_string(completed) + "/" +
                                      std::to_string(files.size()) + "] cached " + file);
            continue;
        }
        jobs.push_back({index, driver::AnalysisCache::NormalizePath(file).string(),
                        std::move(command), sourceStamp, commandSignature});
    }

    std::set<std::string> workingDirectories;
    for (const auto& job : jobs) workingDirectories.insert(job.command.Directory);
    const bool sharedWorkingDirectory = workingDirectories.size() <= 1;
    const auto requestedWorkers = options.jobs == 0 ?
        std::max(1u, std::thread::hardware_concurrency()) : options.jobs;
    auto workerCount = std::min(requestedWorkers, jobs.size());
    if (workerCount > 1 && !sharedWorkingDirectory)
    {
        workerCount = 1;
        diagnostics::Logger::Warning(
            "compile commands use multiple working directories; using one worker");
    }

    const auto originalWorkingDirectory = std::filesystem::current_path();
    if (!jobs.empty() && sharedWorkingDirectory)
    {
        std::error_code currentPathError;
        std::filesystem::current_path(jobs.front().command.Directory, currentPathError);
        if (currentPathError)
        {
            diagnostics::Logger::Error("cannot set compile working directory: " +
                                       currentPathError.message());
            return 2;
        }
    }

    std::atomic<std::size_t> nextJob = 0;
    std::mutex cacheMutex;
    const auto analysisStart = std::chrono::steady_clock::now();
    auto analyze = [&]() {
        while (true)
        {
            const auto jobIndex = nextJob.fetch_add(1, std::memory_order_relaxed);
            if (jobIndex >= jobs.size()) return;
            const auto& job = jobs[jobIndex];
            auto& fileResult = results[job.fileIndex];
            const auto fileStart = std::chrono::steady_clock::now();
            try
            {
                if (!sharedWorkingDirectory)
                {
                    std::error_code currentPathError;
                    std::filesystem::current_path(job.command.Directory, currentPathError);
                    if (currentPathError)
                    {
                        throw std::filesystem::filesystem_error(
                            "cannot set compile working directory", job.command.Directory,
                            currentPathError);
                    }
                }
                DiagnosticsSink analyzedFile;
                driver::DependencyPaths dependencies;
                SingleCommandDatabase singleDatabase(job.command);
                clang::tooling::ClangTool tool(singleDatabase, {job.file});
                driver::ActionFactory factory(config, analyzedFile, options.strict,
                                              sourceRoots, dependencies);
                fileResult.status = tool.run(&factory);
                fileResult.diagnostics = analyzedFile.Diagnostics();
                if (dependencies.empty()) dependencies.emplace_back(job.file);
                if (fileResult.status == 0 && !cachePath.empty())
                {
                    std::lock_guard<std::mutex> lock(cacheMutex);
                    cache.Store(job.file, job.sourceStamp, job.commandSignature,
                                dependencies, fileResult.diagnostics);
                }
            }
            catch (const std::exception& exception)
            {
                fileResult.status = 1;
                diagnostics::Logger::Error("analysis failed for " + job.file + ": " +
                                           exception.what());
            }
            catch (...)
            {
                fileResult.status = 1;
                diagnostics::Logger::Error("analysis failed for " + job.file +
                                           " with an unknown exception");
            }

            const auto completed = completedFiles.fetch_add(1, std::memory_order_relaxed) + 1;
            if (fileResult.status == 0 && !cachePath.empty())
            {
                std::lock_guard<std::mutex> lock(cacheMutex);
                if (completed % kCacheSaveInterval == 0)
                {
                    std::string saveError;
                    if (!cache.Save(saveError)) diagnostics::Logger::Warning(saveError);
                }
            }
            const auto elapsed = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - fileStart).count();
            diagnostics::Logger::Info("[" + std::to_string(completed) + "/" +
                std::to_string(files.size()) + "] completed " + job.file + " (" +
                std::to_string(fileResult.diagnostics.size()) + " diagnostics, " +
                std::to_string(elapsed) + " ms)");
        }
    };

    std::vector<std::thread> workers;
    workers.reserve(workerCount);
    for (std::size_t index = 0; index < workerCount; ++index) workers.emplace_back(analyze);
    for (auto& worker : workers) worker.join();

    if (!jobs.empty())
    {
        std::error_code restoreError;
        std::filesystem::current_path(originalWorkingDirectory, restoreError);
        if (restoreError)
        {
            diagnostics::Logger::Error("cannot restore working directory: " +
                                       restoreError.message());
            return 2;
        }
    }

    for (const auto& fileResult : results)
    {
        result = std::max(result, fileResult.status);
        for (const auto& diagnostic : fileResult.diagnostics) diagnostics.Report(diagnostic);
    }
    diagnostics.SortAndDeduplicate();

    const auto totalElapsed = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - analysisStart).count();
    diagnostics::Logger::Info("analysis workers: " + std::to_string(workerCount) +
        ", cache hits: " + std::to_string(cacheHits) + ", misses: " +
        std::to_string(jobs.size()) + ", elapsed: " + std::to_string(totalElapsed) + " s");

    diagnostics.WriteHuman();
    if (!reportPath.empty() &&
        !JsonReportWriter::Write(reportPath, diagnostics.Diagnostics(), error))
    {
        diagnostics::Logger::Error(error);
        return 2;
    }
    if (!cachePath.empty() && !cache.Save(error))
    {
        diagnostics::Logger::Error(error);
        return 2;
    }
    diagnostics::Logger::Info("analysis complete");
    return result == 0 && !diagnostics.HasErrors() ? 0 : 1;
}
}
