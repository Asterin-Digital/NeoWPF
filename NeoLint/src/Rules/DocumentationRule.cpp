#include <NeoLint/Rules/DocumentationRule.h>

#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>
#include <NeoLint/Core/Suppression.h>

#include <clang/AST/DeclCXX.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Basic/SourceManager.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <sstream>
#include <system_error>
#include <vector>

namespace neolint
{
using namespace clang::ast_matchers;

namespace
{
bool IsHeaderPath(const std::filesystem::path& path)
{
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return extension == ".h";
}

bool IsDocumented(const clang::CXXMethodDecl& method, clang::SourceManager& sourceManager)
{
    const auto location = sourceManager.getFileLoc(method.getBeginLoc());
    if (location.isInvalid()) return false;

    const auto decomposed = sourceManager.getDecomposedLoc(location);
    const auto buffer = sourceManager.getBufferData(decomposed.first);
    const std::string_view source(buffer.data(), buffer.size());
    auto commentEnd = static_cast<std::size_t>(decomposed.second);
    while (commentEnd > 0 && std::isspace(static_cast<unsigned char>(source[commentEnd - 1])))
    {
        --commentEnd;
    }
    if (commentEnd < 2 || source.substr(commentEnd - 2, 2) != "*/") return false;

    const auto commentStart = source.rfind("/*", commentEnd - 2);
    return commentStart != std::string_view::npos &&
        source.substr(commentStart, commentEnd - commentStart).find("@summary") !=
            std::string_view::npos;
}
}

std::string_view DocumentationRule::Id() const noexcept
{
    return "neowpf-documentation";
}

std::string_view DocumentationRule::Description() const noexcept
{
    return "Require paired Set*/Get* documentation and reject @note/@todo in headers.";
}

RuleSeverity DocumentationRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

void DocumentationRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(cxxMethodDecl(unless(isImplicit())).bind("set-get-method"), &callback);
}

void DocumentationRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    const auto* method = result.Nodes.getNodeAs<clang::CXXMethodDecl>("set-get-method");
    if (!method || !method->getName().starts_with("Set")) return;

    const auto* parent = method->getParent();
    if (!parent) return;

    auto& sourceManager = context.SourceMgr();
    const auto location = sourceManager.getExpansionLoc(method->getLocation());
    if (location.isInvalid() || sourceManager.isInSystemHeader(location) ||
        !IsHeaderPath(sourceManager.getFilename(location).str())) return;

    struct MethodGroup
    {
        std::vector<const clang::CXXMethodDecl*> setters;
        std::vector<const clang::CXXMethodDecl*> getters;
        bool hasDocumentedSetter = false;
        bool hasDocumentedGetter = false;
    };

    std::map<std::string, MethodGroup> groups;
    for (const auto* candidate : parent->methods())
    {
        const auto name = candidate->getNameAsString();
        if (name.starts_with("Set") && name.size() > 3)
        {
            auto& group = groups[name.substr(3)];
            group.setters.push_back(candidate);
            group.hasDocumentedSetter |= IsDocumented(*candidate, sourceManager);
        }
        else if (name.starts_with("Get") && name.size() > 3)
        {
            auto& group = groups[name.substr(3)];
            group.getters.push_back(candidate);
            group.hasDocumentedGetter |= IsDocumented(*candidate, sourceManager);
        }
    }

    for (const auto& [suffix, group] : groups)
    {
        if (group.setters.empty() || group.getters.empty()) continue;
        const auto reportUndocumented = [&](
            const std::vector<const clang::CXXMethodDecl*>& methods, bool hasDocumentation,
            std::string_view counterpart)
        {
            if (!hasDocumentation) return;
            for (const auto* candidate : methods)
            {
                if (IsDocumented(*candidate, sourceManager)) continue;
                context.Report(Id(), candidate->getLocation(), context.SettingsFor(Id()).severity,
                    candidate->getNameAsString() + " is undocumented while " +
                    std::string(counterpart) + suffix + " is documented.");
            }
        };

        reportUndocumented(group.setters, group.hasDocumentedGetter, "Get");
        reportUndocumented(group.getters, group.hasDocumentedSetter, "Set");
    }
}

void DocumentationRule::Run(
    const std::vector<std::filesystem::path>& sourceRoots,
    const RuleConfig& config, DiagnosticsSink& diagnostics)
{
    for (const auto& root : sourceRoots)
    {
        std::error_code error;
        std::filesystem::recursive_directory_iterator entry(root, error), end;
        while (!error && entry != end)
        {
            const auto path = entry->path();
            if (entry->is_regular_file(error) && IsHeaderPath(path))
            {
                auto settings = config.SettingsFor(Id(), path);
                if (!config.HasRule(Id())) settings.severity = DefaultSeverity();
                if (settings.enabled)
                {
                    std::ifstream input(path, std::ios::binary);
                    const std::string source(
                        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>{});
                    std::istringstream sourceLines(source);
                    std::string line;
                    std::size_t lineNumber = 0;
                    while (std::getline(sourceLines, line))
                    {
                        ++lineNumber;
                        for (const auto tag : {std::string_view("@note"), std::string_view("@todo")})
                        {
                            if (line.find(tag) == std::string::npos ||
                                Suppression::IsSuppressed(Id(), source, lineNumber)) continue;
                            diagnostics.Report({std::string(Id()), settings.severity,
                                {path.generic_string(), static_cast<std::uint32_t>(lineNumber), 1},
                                std::string(tag) + " is not allowed in header files."});
                        }
                    }
                }
            }
            entry.increment(error);
        }
    }
}

namespace
{
const RuleRegistrar<DocumentationRule> astRegistrar;
const TreeRuleRegistrar<DocumentationRule> treeRegistrar;
}
}