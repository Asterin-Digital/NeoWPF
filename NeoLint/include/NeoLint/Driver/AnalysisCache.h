#pragma once

#include <NeoLint/Core/RuleDiagnostic.h>

#include <clang/Tooling/CompilationDatabase.h>

#include <filesystem>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace neolint::driver
{
struct DependencyStamp
{
    std::string path;
    std::uintmax_t size = 0;
    std::int64_t modified = 0;
    std::string contentHash;
};

class AnalysisCache
{
public:
    static AnalysisCache Load(const std::filesystem::path& path,
                              const std::filesystem::path& config,
                              const std::filesystem::path& executable);

    bool TryGet(const std::filesystem::path& file,
                const std::string& sourceStamp,
                const std::string& commandSignature,
                DiagnosticList& diagnostics);
    void Store(const std::filesystem::path& file,
               const std::string& sourceStamp,
               const std::string& commandSignature,
               const std::vector<std::filesystem::path>& dependencies,
               const DiagnosticList& diagnostics);
    void Prune(const std::vector<std::filesystem::path>& files);
    bool Save(std::string& error) const;
    std::size_t EntryCount() const noexcept;

    static std::filesystem::path NormalizePath(const std::filesystem::path& path);
    static std::string FileStamp(const std::filesystem::path& path);
    static std::string CommandSignature(
        const std::vector<clang::tooling::CompileCommand>& commands);

private:
    struct Entry
    {
        std::string sourceStamp;
        std::string commandSignature;
        std::vector<DependencyStamp> dependencies;
        DiagnosticList diagnostics;
    };

    explicit AnalysisCache(std::filesystem::path path);

    std::filesystem::path path_;
    std::string configStamp_;
    std::string toolIdentity_;
    std::unordered_map<std::string, DependencyStamp> dependencySnapshotCache_;
    std::unordered_map<std::string, Entry> entries_;
};
}
