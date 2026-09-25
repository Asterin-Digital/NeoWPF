#include <NeoLint/Driver/AnalysisCache.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>

namespace neolint::driver
{
namespace
{
using Json = nlohmann::json;

std::unordered_map<std::string, std::string> HeaderStamps(
    const std::vector<std::filesystem::path>& roots)
{
    std::unordered_map<std::string, std::string> stamps;
    for (const auto& root : roots)
    {
        std::error_code error;
        if (!std::filesystem::exists(root, error)) continue;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root, error))
        {
            if (error || !entry.is_regular_file(error)) continue;
            const auto extension = entry.path().extension().string();
            if (extension != ".h" && extension != ".hh" && extension != ".hpp" &&
                extension != ".hxx" && extension != ".inl" && extension != ".inc") continue;
            const auto path = AnalysisCache::NormalizePath(entry.path());
            stamps[path.generic_string()] = AnalysisCache::FileStamp(path);
        }
    }
    return stamps;
}

Json DiagnosticsToJson(const DiagnosticList& diagnostics)
{
    auto result = Json::array();
    for (const auto& diagnostic : diagnostics)
    {
        result.push_back({
            {"rule", diagnostic.ruleId},
            {"severity", diagnostic.severity == RuleSeverity::Error ? "error" :
                diagnostic.severity == RuleSeverity::Warning ? "warning" : "info"},
            {"file", diagnostic.location.file},
            {"line", diagnostic.location.line},
            {"column", diagnostic.location.column},
            {"message", diagnostic.message},
        });
    }
    return result;
}

DiagnosticList DiagnosticsFromJson(const Json& values)
{
    DiagnosticList diagnostics;
    if (!values.is_array()) return diagnostics;
    for (const auto& value : values)
    {
        if (!value.is_object() || !value.contains("rule") || !value.contains("severity") ||
            !value.contains("file") || !value.contains("line") || !value.contains("column") ||
            !value.contains("message")) continue;
        RuleDiagnostic diagnostic;
        diagnostic.ruleId = value["rule"].get<std::string>();
        const auto severity = value["severity"].get<std::string>();
        diagnostic.severity = severity == "error" ? RuleSeverity::Error :
            severity == "warning" ? RuleSeverity::Warning : RuleSeverity::Info;
        diagnostic.location.file = value["file"].get<std::string>();
        diagnostic.location.line = value["line"].get<unsigned>();
        diagnostic.location.column = value["column"].get<unsigned>();
        diagnostic.message = value["message"].get<std::string>();
        diagnostics.push_back(std::move(diagnostic));
    }
    return diagnostics;
}
}

AnalysisCache::AnalysisCache(std::filesystem::path path) : path_(std::move(path)) {}

std::filesystem::path AnalysisCache::NormalizePath(const std::filesystem::path& path)
{
    std::error_code error;
    const auto normalized = std::filesystem::weakly_canonical(path, error);
    return error ? std::filesystem::absolute(path) : normalized;
}

std::string AnalysisCache::FileStamp(const std::filesystem::path& path)
{
    std::error_code error;
    const auto time = std::filesystem::last_write_time(path, error);
    if (error) return {};
    return std::to_string(time.time_since_epoch().count());
}

std::string AnalysisCache::CommandSignature(
    const std::vector<clang::tooling::CompileCommand>& commands)
{
    std::ostringstream signature;
    for (const auto& command : commands)
    {
        signature << command.Directory << '\n';
        for (const auto& argument : command.CommandLine) signature << argument << '\n';
    }
    return signature.str();
}

AnalysisCache AnalysisCache::Load(const std::filesystem::path& path,
                                   const std::filesystem::path& config,
                                   const std::vector<std::filesystem::path>& sourceRoots)
{
    AnalysisCache cache(path);
    cache.configStamp_ = FileStamp(config);
    cache.headerStamps_ = HeaderStamps(sourceRoots);
    Json document;
    std::ifstream input(path);
    if (!input) return cache;
    try { input >> document; } catch (const Json::exception&) { return cache; }

    if (document.value("version", 0) != 1 ||
        document.value("config", "") != cache.configStamp_ ||
        document.value("headers", Json{}) != cache.headerStamps_) return cache;

    const auto files = document.value("files", Json::object());
    if (!files.is_object()) return cache;
    for (const auto& [file, value] : files.items())
    {
        if (!value.is_object()) continue;
        cache.sourceStamps_[file] = value.value("source", "");
        cache.commandSignatures_[file] = value.value("command", "");
        cache.diagnostics_[file] = DiagnosticsFromJson(value.value("diagnostics", Json::array()));
    }
    return cache;
}

std::size_t AnalysisCache::EntryCount() const noexcept
{
    return diagnostics_.size();
}

bool AnalysisCache::TryGet(const std::filesystem::path& file,
                           const std::string& sourceStamp,
                           const std::string& commandSignature,
                           DiagnosticList& diagnostics) const
{
    const auto key = NormalizePath(file).generic_string();
    const auto source = sourceStamps_.find(key);
    const auto command = commandSignatures_.find(key);
    const auto cached = diagnostics_.find(key);
    if (source == sourceStamps_.end() || command == commandSignatures_.end() ||
        cached == diagnostics_.end() || source->second != sourceStamp ||
        command->second != commandSignature) return false;
    diagnostics = cached->second;
    return true;
}

void AnalysisCache::Store(const std::filesystem::path& file,
                          const std::string& sourceStamp,
                          const std::string& commandSignature,
                          const DiagnosticList& diagnostics)
{
    const auto key = NormalizePath(file).generic_string();
    sourceStamps_[key] = sourceStamp;
    commandSignatures_[key] = commandSignature;
    diagnostics_[key] = diagnostics;
}

bool AnalysisCache::Save(std::string& error) const
{
    Json document{{"version", 1}, {"config", configStamp_}, {"headers", headerStamps_},
                  {"files", Json::object()}};
    for (const auto& [file, diagnostics] : diagnostics_)
    {
        document["files"][file] = {
            {"source", sourceStamps_.at(file)},
            {"command", commandSignatures_.at(file)},
            {"diagnostics", DiagnosticsToJson(diagnostics)},
        };
    }

    std::ofstream output(path_, std::ios::trunc);
    if (!output)
    {
        error = "cannot write cache '" + path_.string() + "'";
        return false;
    }
    output << document.dump() << '\n';
    if (!output.good())
    {
        error = "cannot finish writing cache '" + path_.string() + "'";
        return false;
    }
    return true;
}
}
