#pragma once

#include <NeoLint/Core/RuleDiagnostic.h>

#include <clang/Tooling/CompilationDatabase.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace neolint::driver
{
class AnalysisCache
{
public:
    static AnalysisCache Load(const std::filesystem::path& path,
                              const std::filesystem::path& config,
                              const std::vector<std::filesystem::path>& sourceRoots);

    bool TryGet(const std::filesystem::path& file,
                const std::string& sourceStamp,
                const std::string& commandSignature,
                DiagnosticList& diagnostics) const;
    void Store(const std::filesystem::path& file,
               const std::string& sourceStamp,
               const std::string& commandSignature,
               const DiagnosticList& diagnostics);
    bool Save(std::string& error) const;
    std::size_t EntryCount() const noexcept;

    static std::filesystem::path NormalizePath(const std::filesystem::path& path);
    static std::string FileStamp(const std::filesystem::path& path);
    static std::string CommandSignature(
        const std::vector<clang::tooling::CompileCommand>& commands);

private:
    explicit AnalysisCache(std::filesystem::path path);

    std::filesystem::path path_;
    std::string configStamp_;
    std::unordered_map<std::string, std::string> headerStamps_;
    std::unordered_map<std::string, std::string> sourceStamps_;
    std::unordered_map<std::string, std::string> commandSignatures_;
    std::unordered_map<std::string, DiagnosticList> diagnostics_;
};
}
