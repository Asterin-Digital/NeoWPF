#include <NeoLint/Rules/PlatformDependencyIsolationRule.h>

#include <NeoLint/Core/DiagnosticsSink.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>
#include <NeoLint/Core/Suppression.h>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <iterator>
#include <optional>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace neolint
{
using namespace clang::ast_matchers;

namespace
{
using PlatformSet = std::set<std::string, std::less<>>;

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

PlatformSet ParsePlatforms(std::string_view value)
{
    PlatformSet platforms;
    std::size_t start = 0;
    while (start < value.size())
    {
        const auto end = value.find(',', start);
        auto platform = std::string(value.substr(start,
            end == std::string_view::npos ? value.size() - start : end - start));
        const auto first = platform.find_first_not_of(" \t");
        if (first != std::string::npos)
        {
            const auto last = platform.find_last_not_of(" \t");
            platforms.insert(Lower(platform.substr(first, last - first + 1)));
        }
        if (end == std::string_view::npos) break;
        start = end + 1;
    }
    return platforms;
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

bool MatchesPathPattern(std::string_view path, std::string_view pattern)
{
    const auto normalizedPattern = NormalizePath(std::string(pattern));
    if (MatchesPattern(path, normalizedPattern)) return true;
    if (normalizedPattern.find('/') == std::string::npos)
    {
        const auto filename = path.substr(path.find_last_of('/') == std::string_view::npos ?
            0 : path.find_last_of('/') + 1);
        return MatchesPattern(filename, normalizedPattern);
    }
    return false;
}

PlatformSet DefaultPlatforms()
{
    return {"windows", "linux", "android"};
}

PlatformSet ConfiguredPlatforms(const RuleOptions& options)
{
    PlatformSet result;
    for (const auto& platform : options.GetStrings("platforms"))
    {
        result.insert(Lower(platform));
    }
    return result.empty() ? DefaultPlatforms() : result;
}

PlatformSet BuiltInHeaderPlatforms(std::string_view path)
{
    const auto filenameStart = path.find_last_of('/');
    const auto filename = filenameStart == std::string_view::npos ?
        path : path.substr(filenameStart + 1);
    constexpr std::array windowsHeaders{
        "windows.h", "windef.h", "winnt.h", "winbase.h", "winuser.h", "fileapi.h",
        "processthreadsapi.h", "synchapi.h", "handleapi.h", "errhandlingapi.h",
        "libloaderapi.h", "shellapi.h", "shlobj.h", "unknwn.h", "combaseapi.h",
    };
    constexpr std::array posixHeaders{
        "unistd.h", "pthread.h", "dirent.h", "dlfcn.h", "termios.h", "semaphore.h",
        "arpa/inet.h", "netinet/in.h", "sys/socket.h", "sys/stat.h", "sys/types.h",
        "sys/wait.h", "sys/mman.h",
    };
    constexpr std::array linuxHeaders{
        "linux/", "asm/", "sys/epoll.h", "sys/eventfd.h", "sys/inotify.h",
    };
    constexpr std::array androidHeaders{
        "android/", "android_native_app_glue.h", "jni.h",
    };
    for (const auto* header : windowsHeaders)
    {
        if (filename == header) return {"windows"};
    }
    for (const auto* header : posixHeaders)
    {
        if (path.ends_with(header)) return {"linux", "android"};
    }
    for (const auto* header : linuxHeaders)
    {
        if (path.find(header) != std::string_view::npos) return {"linux"};
    }
    for (const auto* header : androidHeaders)
    {
        if (path.find(header) != std::string_view::npos) return {"android"};
    }
    if (path.find("/d3d") != std::string_view::npos ||
        path.find("/dxgi") != std::string_view::npos ||
        path.find("/wrl/") != std::string_view::npos ||
        path.find("/winrt/") != std::string_view::npos)
    {
        return {"windows"};
    }
    return {};
}

class PlatformBoundaryClassifier
{
public:
    explicit PlatformBoundaryClassifier(const RuleOptions& options)
        : platforms_(ConfiguredPlatforms(options)),
          pathPlatforms_(options.GetStringMap("path_platforms")),
          headerPlatforms_(options.GetStringMap("header_platforms")),
          apiPlatforms_(options.GetStringMap("api_platforms"))
    {
    }

    PlatformSet PlatformsForPath(std::string path) const
    {
        path = NormalizePath(std::move(path));
        if (const auto configured = FindPathMatch(path, pathPlatforms_)) return *configured;
        if (const auto configured = FindPathMatch(path, headerPlatforms_)) return *configured;
        const auto builtIn = BuiltInHeaderPlatforms(path);
        return builtIn.empty() ? platforms_ : builtIn;
    }

    PlatformSet PlatformsForDeclaration(const clang::NamedDecl& declaration) const
    {
        const auto qualifiedName = declaration.getQualifiedNameAsString();
        if (const auto configured = FindNameMatch(qualifiedName, apiPlatforms_)) return *configured;
        if (const auto configured = FindNameMatch(declaration.getNameAsString(), apiPlatforms_))
        {
            return *configured;
        }
        return PlatformsForPath(DeclarationPath(declaration));
    }

    PlatformSet AllPlatforms() const { return platforms_; }

private:
    static std::optional<PlatformSet> FindPathMatch(
        const std::string& path, const RuleOptions::StringMap& mappings)
    {
        for (const auto& [pattern, value] : mappings)
        {
            if (MatchesPathPattern(path, pattern)) return ParsePlatforms(value);
        }
        return std::nullopt;
    }

    static std::optional<PlatformSet> FindNameMatch(
        const std::string& name, const RuleOptions::StringMap& mappings)
    {
        for (const auto& [pattern, value] : mappings)
        {
            if (MatchesPattern(name, pattern)) return ParsePlatforms(value);
        }
        return std::nullopt;
    }

    static std::string DeclarationPath(const clang::NamedDecl& declaration)
    {
        const auto& sourceManager = declaration.getASTContext().getSourceManager();
        return NormalizePath(sourceManager.getFilename(declaration.getLocation()).str());
    }

    PlatformSet platforms_;
    RuleOptions::StringMap pathPlatforms_;
    RuleOptions::StringMap headerPlatforms_;
    RuleOptions::StringMap apiPlatforms_;
};

std::string FormatPlatforms(const PlatformSet& platforms)
{
    std::string result;
    for (const auto& platform : platforms)
    {
        if (!result.empty()) result += ", ";
        result += platform;
    }
    return result;
}

PlatformSet MissingPlatforms(const PlatformSet& consumer, const PlatformSet& dependency)
{
    PlatformSet missing;
    std::ranges::set_difference(consumer, dependency,
                                std::inserter(missing, missing.end()));
    return missing;
}

std::string IncludeChain(const clang::SourceManager& sourceManager, clang::FileID fileId)
{
    std::vector<std::string> files;
    while (fileId.isValid())
    {
        const auto location = sourceManager.getLocForStartOfFile(fileId);
        const auto filename = sourceManager.getFilename(location);
        if (!filename.empty()) files.push_back(filename.str());
        const auto includeLocation = sourceManager.getIncludeLoc(fileId);
        if (includeLocation.isInvalid()) break;
        fileId = sourceManager.getFileID(includeLocation);
    }
    std::ranges::reverse(files);
    std::string result;
    for (const auto& file : files)
    {
        if (!result.empty()) result += " -> ";
        result += file;
    }
    return result;
}

class PlatformDependencyCallbacks final : public clang::PPCallbacks
{
public:
    PlatformDependencyCallbacks(const RuleOptions& options, const RuleConfig& config,
                                DiagnosticsSink& diagnostics,
                                clang::SourceManager& sourceManager, bool strict)
        : classifier_(options), config_(config), diagnostics_(diagnostics),
          sourceManager_(sourceManager), suppression_(sourceManager), strict_(strict)
    {
    }

    void LexedFileChanged(clang::FileID fileId, LexedFileChangeReason reason,
                          clang::SrcMgr::CharacteristicKind, clang::FileID,
                          clang::SourceLocation) override
    {
        if (reason != LexedFileChangeReason::EnterFile) return;
        const auto includeLocation = sourceManager_.getIncludeLoc(fileId);
        if (includeLocation.isInvalid()) return;
        if (sourceManager_.isInSystemHeader(includeLocation)) return;
        const auto parentFileId = sourceManager_.getFileID(includeLocation);
        const auto parentPath = sourceManager_.getFilename(includeLocation).str();
        const auto childLocation = sourceManager_.getLocForStartOfFile(fileId);
        const auto childPath = sourceManager_.getFilename(childLocation).str();
        const auto missing = MissingPlatforms(classifier_.PlatformsForPath(parentPath),
                                              classifier_.PlatformsForPath(childPath));
        if (!missing.empty())
        {
            Report(includeLocation,
                   "include of '" + childPath + "' is unavailable on " +
                       FormatPlatforms(missing) + "; include chain: " +
                       IncludeChain(sourceManager_, fileId));
        }
    }

private:
    void Report(clang::SourceLocation location, std::string message) const
    {
        const auto presumed = sourceManager_.getPresumedLoc(location);
        if (presumed.isInvalid()) return;
        const auto ruleId = "neowpf-platform-dependency-isolation";
        const auto settings = config_.SettingsFor(ruleId, presumed.getFilename());
        if (!settings.enabled || (settings.severity == RuleSeverity::Info && !strict_) ||
            suppression_.IsSuppressed(ruleId, location))
        {
            return;
        }
        diagnostics_.Report({ruleId, settings.severity,
            {presumed.getFilename(), presumed.getLine(), presumed.getColumn()},
            std::move(message)});
    }

    PlatformBoundaryClassifier classifier_;
    const RuleConfig& config_;
    DiagnosticsSink& diagnostics_;
    clang::SourceManager& sourceManager_;
    Suppression suppression_;
    bool strict_;
};
}

std::string_view PlatformDependencyIsolationRule::Id() const noexcept
{
    return "neowpf-platform-dependency-isolation";
}

std::string_view PlatformDependencyIsolationRule::Description() const noexcept
{
    return "Prevent platform-specific dependencies from crossing configured platform boundaries.";
}

RuleSeverity PlatformDependencyIsolationRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

bool PlatformDependencyIsolationRule::ValidateOptions(
    const RuleOptions& options, std::string& error) const
{
    for (const auto& key : options.Keys())
    {
        const bool valid = (key == "platforms" &&
                options.TypeOf(key) == RuleOptions::Type::StringList) ||
            ((key == "path_platforms" || key == "header_platforms" ||
              key == "api_platforms") &&
                options.TypeOf(key) == RuleOptions::Type::StringMap);
        if (!valid)
        {
            error = "rule '" + std::string(Id()) + "' has invalid option '" + key + "'";
            return false;
        }
    }
    const auto platforms = ConfiguredPlatforms(options);
    if (platforms.empty())
    {
        error = "rule '" + std::string(Id()) + "' requires at least one platform";
        return false;
    }
    const auto validateMappings = [&](std::string_view optionName,
                                      const RuleOptions::StringMap& mappings) {
        for (const auto& [pattern, value] : mappings)
        {
            if (pattern.empty())
            {
                error = "rule '" + std::string(Id()) + "' option '" +
                    std::string(optionName) + "' contains an empty pattern";
                return false;
            }
            const auto mappedPlatforms = ParsePlatforms(value);
            if (mappedPlatforms.empty())
            {
                error = "rule '" + std::string(Id()) + "' option '" +
                    std::string(optionName) + "' has an empty platform set";
                return false;
            }
            for (const auto& platform : mappedPlatforms)
            {
                if (!platforms.contains(platform))
                {
                    error = "rule '" + std::string(Id()) + "' option '" +
                        std::string(optionName) + "' uses unconfigured platform '" +
                        platform + "'";
                    return false;
                }
            }
        }
        return true;
    };
    return validateMappings("path_platforms", options.GetStringMap("path_platforms")) &&
        validateMappings("header_platforms", options.GetStringMap("header_platforms")) &&
        validateMappings("api_platforms", options.GetStringMap("api_platforms"));
}

void PlatformDependencyIsolationRule::Configure(const RuleOptions& options)
{
    options_ = options;
}

void PlatformDependencyIsolationRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(
        declRefExpr(to(namedDecl().bind("platform-dependency-decl")))
            .bind("platform-dependency-use"), &callback);
    finder.addMatcher(
        typeLoc(loc(qualType(hasDeclaration(namedDecl().bind("platform-dependency-decl")))))
            .bind("platform-dependency-type-use"), &callback);
    finder.addMatcher(
        memberExpr(member(namedDecl().bind("platform-dependency-decl")))
            .bind("platform-dependency-member-use"), &callback);
}

void PlatformDependencyIsolationRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    const auto* declaration = result.Nodes.getNodeAs<clang::NamedDecl>(
        "platform-dependency-decl");
    if (!declaration) return;

    clang::SourceLocation useLocation;
    if (const auto* expression = result.Nodes.getNodeAs<clang::DeclRefExpr>(
            "platform-dependency-use"))
    {
        useLocation = expression->getExprLoc();
    }
    else if (const auto* typeLocation = result.Nodes.getNodeAs<clang::TypeLoc>(
                 "platform-dependency-type-use"))
    {
        useLocation = typeLocation->getBeginLoc();
    }
    else if (const auto* member = result.Nodes.getNodeAs<clang::MemberExpr>(
                 "platform-dependency-member-use"))
    {
        useLocation = member->getMemberLoc();
    }
    if (useLocation.isInvalid() || declaration->getLocation().isInvalid()) return;
    auto& sourceManager = context.SourceMgr();
    useLocation = sourceManager.getExpansionLoc(useLocation);
    if (sourceManager.isInSystemHeader(useLocation)) return;

    const PlatformBoundaryClassifier classifier(options_);
    const auto consumerPath = sourceManager.getFilename(useLocation).str();
    const auto consumerPlatforms = classifier.PlatformsForPath(consumerPath);
    const auto dependencyPlatforms = classifier.PlatformsForDeclaration(*declaration);
    const auto missing = MissingPlatforms(consumerPlatforms, dependencyPlatforms);
    if (missing.empty()) return;

    auto message = "use of '" + declaration->getQualifiedNameAsString() +
        "' depends on a declaration unavailable on " + FormatPlatforms(missing);
    const auto declarationFile = sourceManager.getFileID(declaration->getLocation());
    const auto chain = IncludeChain(sourceManager, declarationFile);
    if (!chain.empty()) message += "; include chain: " + chain;
    context.Report(Id(), useLocation, context.SettingsFor(Id()).severity, std::move(message));
}

std::unique_ptr<clang::PPCallbacks>
PlatformDependencyIsolationRule::CreatePreprocessorCallbacks(
    const RuleOptions& options, const RuleConfig& config, DiagnosticsSink& diagnostics,
    clang::SourceManager& sourceManager, bool strict)
{
    return std::make_unique<PlatformDependencyCallbacks>(
        options, config, diagnostics, sourceManager, strict);
}

namespace
{
const RuleRegistrar<PlatformDependencyIsolationRule> registrar;
}
}