#include <NeoLint/Rules/PlatformCodeLocationRule.h>

#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>
#include <NeoLint/Core/Suppression.h>

#include <clang/AST/Decl.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Basic/SourceManager.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace neolint
{
using namespace clang::ast_matchers;

namespace
{
using StringMap = RuleOptions::StringMap;

std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

std::string NormalizePath(std::string value)
{
    std::replace(value.begin(), value.end(), '\\', '/');
    return Lower(std::move(value));
}

std::string Trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool MatchesPattern(std::string_view value, std::string_view pattern)
{
    std::size_t valueIndex = 0;
    std::size_t patternIndex = 0;
    std::size_t starIndex = std::string_view::npos;
    std::size_t matchIndex = 0;
    while (valueIndex < value.size())
    {
        if (patternIndex < pattern.size() &&
            (pattern[patternIndex] == '?' || pattern[patternIndex] == value[valueIndex]))
        {
            ++valueIndex;
            ++patternIndex;
        }
        else if (patternIndex < pattern.size() && pattern[patternIndex] == '*')
        {
            starIndex = patternIndex++;
            matchIndex = valueIndex;
        }
        else if (starIndex != std::string_view::npos)
        {
            patternIndex = starIndex + 1;
            valueIndex = ++matchIndex;
        }
        else
        {
            return false;
        }
    }
    while (patternIndex < pattern.size() && pattern[patternIndex] == '*') ++patternIndex;
    return patternIndex == pattern.size();
}

bool IsPlatformDirectory(std::string_view path, std::string_view platform)
{
    const auto normalizedPath = NormalizePath(std::string(path));
    const auto normalizedPlatform = Lower(std::string(platform));
    const auto platformPath = "platform/" + normalizedPlatform + "/";
    return normalizedPath.starts_with(platformPath) ||
        normalizedPath.find("/" + platformPath) != std::string::npos;
}

    bool IsUnderPlatformRoot(std::string_view path)
    {
        const auto normalizedPath = NormalizePath(std::string(path));
        return normalizedPath.starts_with("platform/") ||
        normalizedPath.find("/platform/") != std::string::npos;
    }

bool TokenPresent(std::string_view line, std::string_view token)
{
    auto position = line.find(token);
    while (position != std::string_view::npos)
    {
        const auto isIdentifier = [](char character) {
            return std::isalnum(static_cast<unsigned char>(character)) || character == '_';
        };
        const bool leftBoundary = position == 0 || !isIdentifier(line[position - 1]);
        const auto end = position + token.size();
        const bool rightBoundary = end == line.size() || !isIdentifier(line[end]);
        if (leftBoundary && rightBoundary) return true;
        position = line.find(token, position + 1);
    }
    return false;
}

bool IsPositivePlatformDirective(std::string_view line)
{
    auto directive = Trim(std::string(line));
    if (directive.empty() || directive.front() != '#') return false;
    directive = Trim(directive.substr(1));
    if (!directive.starts_with("if") || directive.starts_with("ifndef")) return false;
    return true;
}

bool GuardContainsDefinition(const std::vector<std::string>& lines, std::size_t guardLine)
{
    std::size_t nestedConditions = 0;
    for (auto index = guardLine + 1; index < lines.size(); ++index)
    {
        const auto line = Trim(lines[index]);
        if (line.empty() || line.starts_with("//") || line.starts_with("/*")) continue;
        if (line.starts_with("#if"))
        {
            ++nestedConditions;
            continue;
        }
        if (line.starts_with("#endif"))
        {
            if (nestedConditions == 0) return false;
            --nestedConditions;
            continue;
        }
        if (line.starts_with("#include") || line.starts_with("#pragma") ||
            line.starts_with("#else") || line.starts_with("#elif"))
        {
            continue;
        }
        if (line.starts_with("#define")) return true;
        if (line.starts_with('#')) continue;
        return true;
    }
    return false;
}

std::vector<std::string> ReadLines(const std::filesystem::path& path)
{
    std::ifstream input(path);
    std::vector<std::string> lines;
    for (std::string line; std::getline(input, line);) lines.push_back(std::move(line));
    return lines;
}

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

std::optional<std::string> PlatformForPath(std::string_view path, const StringMap& mappings)
{
    const auto normalizedPath = NormalizePath(std::string(path));
    for (const auto& [pattern, platform] : mappings)
    {
        if (MatchesPattern(normalizedPath, NormalizePath(pattern))) return Lower(platform);
    }
    return std::nullopt;
}

std::optional<std::string> PlatformForSymbol(std::string_view name, const StringMap& mappings)
{
    for (const auto& [pattern, platform] : mappings)
    {
        if (MatchesPattern(name, pattern)) return Lower(platform);
    }
    return std::nullopt;
}

StringMap PlatformMacros(const RuleOptions& options)
{
    StringMap macros{
        {"_WIN32", "windows"},
        {"_WIN64", "windows"},
        {"__linux__", "linux"},
        {"__ANDROID__", "android"},
    };
    for (const auto& [macro, platform] : options.GetStringMap("platform_macros"))
    {
        macros.insert_or_assign(macro, Lower(platform));
    }
    return macros;
}

std::string PlatformDirectoryName(std::string_view platform)
{
    auto name = Lower(std::string(platform));
    if (!name.empty()) name.front() = static_cast<char>(std::toupper(
        static_cast<unsigned char>(name.front())));
    return name;
}

void ReportTreeDiagnostic(std::string_view ruleId, const std::filesystem::path& path,
                          std::size_t line, std::string_view source,
                          const RuleSettings& settings, DiagnosticsSink& diagnostics,
                          std::string message)
{
    if (!settings.enabled || Suppression::IsSuppressed(ruleId, source, line)) return;
    diagnostics.Report({std::string(ruleId), settings.severity,
        {path.generic_string(), static_cast<std::uint32_t>(line), 1}, std::move(message)});
}
}

std::string_view PlatformCodeLocationRule::Id() const noexcept
{
    return "neowpf-platform-code-location";
}

std::string_view PlatformCodeLocationRule::Description() const noexcept
{
    return "Require platform-specific definitions to live under Platform/<platform>/.";
}

RuleSeverity PlatformCodeLocationRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Error;
}

bool PlatformCodeLocationRule::ValidateOptions(
    const RuleOptions& options, std::string& error) const
{
    for (const auto& key : options.Keys())
    {
        const bool valid = (key == "platform_macros" || key == "platform_paths" ||
                            key == "platform_symbols") &&
            options.TypeOf(key) == RuleOptions::Type::StringMap;
        if (valid) continue;
        error = "rule '" + std::string(Id()) + "' has invalid option '" + key + "'";
        return false;
    }
    for (const auto& key : options.Keys())
    {
        for (const auto& [pattern, platform] : options.GetStringMap(key))
        {
            if (pattern.empty() || Trim(platform).empty())
            {
                error = "rule '" + std::string(Id()) + "' option '" + key +
                    "' contains an empty mapping";
                return false;
            }
        }
    }
    return true;
}

void PlatformCodeLocationRule::Configure(const RuleOptions& options)
{
    options_ = options;
}

void PlatformCodeLocationRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(functionDecl(isDefinition(), unless(isImplicit()))
                          .bind("platform-code-definition"), &callback);
    finder.addMatcher(cxxRecordDecl(isDefinition(), unless(isImplicit()))
                          .bind("platform-code-definition"), &callback);
    finder.addMatcher(varDecl(isDefinition(), unless(isImplicit()))
                          .bind("platform-code-definition"), &callback);
}

void PlatformCodeLocationRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    const auto* declaration = result.Nodes.getNodeAs<clang::NamedDecl>(
        "platform-code-definition");
    if (!declaration || declaration->getLocation().isInvalid()) return;
    auto& sourceManager = context.SourceMgr();
    const auto location = sourceManager.getExpansionLoc(declaration->getLocation());
    if (sourceManager.isInSystemHeader(location)) return;

    const auto symbols = options_.GetStringMap("platform_symbols");
    const auto qualifiedName = declaration->getQualifiedNameAsString();
    auto platform = PlatformForSymbol(qualifiedName, symbols);
    if (!platform) platform = PlatformForSymbol(declaration->getNameAsString(), symbols);
    if (!platform) return;

    const auto path = sourceManager.getFilename(location).str();
    if (IsPlatformDirectory(path, *platform)) return;
    context.Report(Id(), location, context.SettingsFor(Id()).severity,
        "platform-specific definition '" + qualifiedName + "' belongs under Platform/" +
            PlatformDirectoryName(*platform) + "/");
}

void PlatformCodeLocationRule::Run(
    const std::vector<std::filesystem::path>& sourceRoots,
    const RuleConfig& config, DiagnosticsSink& diagnostics)
{
    for (const auto& root : sourceRoots)
    {
        std::error_code error;
        std::filesystem::recursive_directory_iterator entry(root, error), end;
        while (!error && entry != end)
        {
            if (!entry->is_regular_file(error))
            {
                entry.increment(error);
                continue;
            }

            const auto path = entry->path();
            const auto settings = config.SettingsFor(Id(), path);
            if (!settings.enabled)
            {
                entry.increment(error);
                continue;
            }

            const auto contents = ReadFile(path);
            const auto lines = ReadLines(path);
            if (const auto platform = PlatformForPath(
                    path.generic_string(), settings.options.GetStringMap("platform_paths"));
                platform && !IsPlatformDirectory(path.generic_string(), *platform))
            {
                ReportTreeDiagnostic(Id(), path, 1, contents, settings, diagnostics,
                    "platform-specific file belongs under Platform/" +
                        PlatformDirectoryName(*platform) + "/");
            }

            const auto macros = PlatformMacros(settings.options);
            for (std::size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
            {
                const auto& line = lines[lineIndex];
                if (IsUnderPlatformRoot(path.generic_string()) ||
                    !IsPositivePlatformDirective(line) ||
                    !GuardContainsDefinition(lines, lineIndex))
                {
                    continue;
                }

                std::set<std::string, std::less<>> guardedPlatforms;
                for (const auto& [macro, platform] : macros)
                {
                    if (TokenPresent(line, macro)) guardedPlatforms.insert(platform);
                }
                if (guardedPlatforms.size() != 1) continue;

                const auto& platform = *guardedPlatforms.begin();
                if (IsPlatformDirectory(path.generic_string(), platform)) continue;
                ReportTreeDiagnostic(Id(), path, lineIndex + 1, contents, settings, diagnostics,
                    "platform-specific definition belongs under Platform/" +
                        PlatformDirectoryName(platform) + "/");
            }
            entry.increment(error);
        }
    }
}

namespace
{
const RuleRegistrar<PlatformCodeLocationRule> astRegistrar;
const TreeRuleRegistrar<PlatformCodeLocationRule> treeRegistrar;
}
}