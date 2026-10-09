#include <NeoLint/Driver/AnalysisCache.h>

#include <nlohmann/json.hpp>
#include <llvm/Support/FileSystem.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <set>
#include <sstream>
#include <system_error>
#include <utility>

namespace neolint::driver
{
namespace
{
using Json = nlohmann::json;

std::string ContentHash(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) return {};
    std::uint64_t hash = 14695981039346656037ull;
    std::array<char, 8192> buffer{};
    while (input)
    {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        for (std::streamsize index = 0; index < input.gcount(); ++index)
        {
            hash ^= static_cast<unsigned char>(buffer[static_cast<std::size_t>(index)]);
            hash *= 1099511628211ull;
        }
    }
    if (input.bad()) return {};
    std::ostringstream result;
    result << std::hex << hash;
    return result.str();
}

bool ReadMetadata(const std::filesystem::path& path, std::uintmax_t& size,
                  std::int64_t& modified)
{
    std::error_code error;
    size = std::filesystem::file_size(path, error);
    if (error) return false;
    const auto time = std::filesystem::last_write_time(path, error);
    if (error) return false;
    modified = time.time_since_epoch().count();
    return true;
}

std::vector<DependencyStamp> ReadDependencies(const Json& values)
{
    std::vector<DependencyStamp> dependencies;
    if (!values.is_array()) return dependencies;
    for (const auto& value : values)
    {
        if (!value.is_object()) continue;
        dependencies.push_back({
            value.value("path", std::string{}),
            value.value("size", std::uintmax_t{}),
            value.value("modified", std::int64_t{}),
            value.value("hash", std::string{}),
        });
    }
    return dependencies;
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
    std::uintmax_t size = 0;
    std::int64_t modified = 0;
    if (!ReadMetadata(path, size, modified)) return {};
    return std::to_string(size) + ":" + std::to_string(modified);
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
                                  const std::filesystem::path& executable)
{
    AnalysisCache cache(path);
    cache.configStamp_ = ContentHash(config);
    cache.toolIdentity_ = executable.empty() ? "unavailable" :
        NormalizePath(executable).generic_string() + "|" + FileStamp(executable);
    Json document;
    std::ifstream input(path);
    if (!input) return cache;
    try { input >> document; } catch (const Json::exception&) { return cache; }

    if (document.value("version", 0) != 2 ||
        document.value("config", "") != cache.configStamp_ ||
        document.value("tool", "") != cache.toolIdentity_) return cache;

    const auto files = document.value("files", Json::object());
    if (!files.is_object()) return cache;
    for (const auto& [file, value] : files.items())
    {
        if (!value.is_object()) continue;
        Entry entry;
        entry.sourceStamp = value.value("source", "");
        entry.commandSignature = value.value("command", "");
        entry.dependencies = ReadDependencies(value.value("dependencies", Json::array()));
        entry.diagnostics = DiagnosticsFromJson(value.value("diagnostics", Json::array()));
        cache.entries_.emplace(file, std::move(entry));
    }
    return cache;
}

std::size_t AnalysisCache::EntryCount() const noexcept
{
    return entries_.size();
}

bool AnalysisCache::TryGet(const std::filesystem::path& file,
                           const std::string& sourceStamp,
                           const std::string& commandSignature,
                           DiagnosticList& diagnostics)
{
    const auto key = NormalizePath(file).generic_string();
    const auto cached = entries_.find(key);
    if (cached == entries_.end() || cached->second.sourceStamp != sourceStamp ||
        cached->second.commandSignature != commandSignature) return false;
    for (auto& dependency : cached->second.dependencies)
    {
        auto snapshot = dependencySnapshotCache_.find(dependency.path);
        if (snapshot == dependencySnapshotCache_.end())
        {
            DependencyStamp current;
            current.path = dependency.path;
            if (!ReadMetadata(dependency.path, current.size, current.modified)) return false;
            current.contentHash = current.size == dependency.size &&
                current.modified == dependency.modified ? dependency.contentHash :
                    ContentHash(dependency.path);
            if (current.contentHash.empty()) return false;
            snapshot = dependencySnapshotCache_.emplace(current.path, std::move(current)).first;
        }
        const auto& current = snapshot->second;
        if (current.contentHash != dependency.contentHash) return false;
        dependency.size = current.size;
        dependency.modified = current.modified;
    }
    diagnostics = cached->second.diagnostics;
    return true;
}

void AnalysisCache::Store(const std::filesystem::path& file,
                          const std::string& sourceStamp,
                          const std::string& commandSignature,
                          const std::vector<std::filesystem::path>& dependencies,
                          const DiagnosticList& diagnostics)
{
    const auto key = NormalizePath(file).generic_string();
    Entry entry;
    entry.sourceStamp = sourceStamp;
    entry.commandSignature = commandSignature;
    entry.diagnostics = diagnostics;
    std::set<std::string> seen;
    for (const auto& dependency : dependencies)
    {
        const auto normalized = NormalizePath(dependency);
        const auto dependencyKey = normalized.generic_string();
        if (!seen.insert(dependencyKey).second) continue;
        auto cachedStamp = dependencySnapshotCache_.find(dependencyKey);
        if (cachedStamp == dependencySnapshotCache_.end())
        {
            DependencyStamp stamp;
            stamp.path = dependencyKey;
            if (!ReadMetadata(normalized, stamp.size, stamp.modified)) continue;
            stamp.contentHash = ContentHash(normalized);
            if (stamp.contentHash.empty()) continue;
            cachedStamp = dependencySnapshotCache_.emplace(
                dependencyKey, std::move(stamp)).first;
        }
        auto stamp = cachedStamp->second;
        entry.dependencies.push_back(std::move(stamp));
    }
    entries_.insert_or_assign(key, std::move(entry));
}

void AnalysisCache::Prune(const std::vector<std::filesystem::path>& files)
{
    std::set<std::string> selected;
    for (const auto& file : files) selected.insert(NormalizePath(file).generic_string());
    std::erase_if(entries_, [&](const auto& entry) { return !selected.contains(entry.first); });
    dependencySnapshotCache_.clear();
}

bool AnalysisCache::Save(std::string& error) const
{
    Json document{{"version", 2}, {"config", configStamp_}, {"tool", toolIdentity_},
                  {"files", Json::object()}};
    for (const auto& [file, entry] : entries_)
    {
        auto dependencies = Json::array();
        for (const auto& dependency : entry.dependencies)
        {
            dependencies.push_back({{"path", dependency.path}, {"size", dependency.size},
                {"modified", dependency.modified}, {"hash", dependency.contentHash}});
        }
        document["files"][file] = {
            {"source", entry.sourceStamp},
            {"command", entry.commandSignature},
            {"dependencies", std::move(dependencies)},
            {"diagnostics", DiagnosticsToJson(entry.diagnostics)},
        };
    }

    std::error_code directoryError;
    if (!path_.parent_path().empty())
    {
        std::filesystem::create_directories(path_.parent_path(), directoryError);
        if (directoryError)
        {
            error = "cannot create cache directory '" + path_.parent_path().string() + "'";
            return false;
        }
    }
    auto temporaryPath = path_;
    temporaryPath += ".tmp";
    std::ofstream output(temporaryPath, std::ios::trunc);
    if (!output)
    {
        error = "cannot write cache '" + temporaryPath.string() + "'";
        return false;
    }
    output << document.dump() << '\n';
    output.flush();
    if (!output.good())
    {
        error = "cannot finish writing cache '" + temporaryPath.string() + "'";
        return false;
    }
    output.close();
    const auto renameError = llvm::sys::fs::rename(temporaryPath.string(), path_.string());
    if (renameError)
    {
        error = "cannot replace cache '" + path_.string() + "': " + renameError.message();
        return false;
    }
    return true;
}
}
