#include <NeoLint/Rules/DocumentationRule.h>

#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>
#include <NeoLint/Core/Suppression.h>

#include <clang/AST/Decl.h>
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
#include <unordered_map>
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

bool IsDocumented(const clang::Decl& declaration, clang::SourceManager& sourceManager)
{
    const auto location = sourceManager.getFileLoc(declaration.getBeginLoc());
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
    if (commentStart == std::string_view::npos) return false;
    const auto comment = source.substr(commentStart, commentEnd - commentStart);
    return comment.find("@summary") != std::string_view::npos ||
        comment.find("@internal") != std::string_view::npos;
}

std::string HeaderPath(const clang::Decl& declaration, clang::SourceManager& sourceManager)
{
    const auto location = sourceManager.getExpansionLoc(declaration.getLocation());
    if (location.isInvalid() || sourceManager.isInSystemHeader(location)) return {};
    const auto path = std::filesystem::path(sourceManager.getFilename(location).str());
    return IsHeaderPath(path) ? path.generic_string() : std::string{};
}

bool IsPublicClass(const clang::CXXRecordDecl& record)
{
    const auto access = record.getAccess();
    return record.isExternallyVisible() &&
        (access == clang::AS_none || access == clang::AS_public);
}

bool IsDocumentableMethod(const clang::CXXMethodDecl& method)
{
    if (method.isImplicit() || llvm::isa<clang::CXXConstructorDecl>(method) ||
        llvm::isa<clang::CXXDestructorDecl>(method) ||
        method.isOverloadedOperator()) return false;

    const auto returnType = method.getReturnType().getNonReferenceType();
    const auto* returnRecord = returnType->getAsCXXRecordDecl();
    if (method.isStatic() && method.getName().ends_with("Property") && returnRecord &&
        returnRecord->getName() == "DependencyProperty") return false;

    const auto access = method.getAccess();
    return access == clang::AS_public ||
        (access == clang::AS_none && method.getParent()->isStruct());
}

struct ClassDocumentation
{
    const clang::CXXRecordDecl* definition = nullptr;
    bool hasDocumentation = false;
};

struct FileDeclarations
{
    bool hasClass = false;
    std::map<const clang::CXXRecordDecl*, ClassDocumentation> classes;
    std::vector<const clang::FunctionDecl*> functions;
};

void CollectDeclarations(const clang::DeclContext& declarationContext,
                         clang::SourceManager& sourceManager,
                         std::unordered_map<std::string, FileDeclarations>& files)
{
    for (const auto* declaration : declarationContext.decls())
    {
        if (declaration->isImplicit()) continue;

        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(declaration))
        {
            const auto path = HeaderPath(*record, sourceManager);
            if (!path.empty())
            {
                auto& file = files[path];
                file.hasClass = true;
                auto& classInfo = file.classes[record->getCanonicalDecl()];
                if (record->isThisDeclarationADefinition()) classInfo.definition = record;
                classInfo.hasDocumentation |= IsDocumented(*record, sourceManager);
            }
        }
        else if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(declaration))
        {
            if (!llvm::isa<clang::CXXMethodDecl>(function))
            {
                const auto path = HeaderPath(*function, sourceManager);
                if (!path.empty()) files[path].functions.push_back(function);
            }
        }

        if (const auto* nestedContext = llvm::dyn_cast<clang::DeclContext>(declaration))
        {
            CollectDeclarations(*nestedContext, sourceManager, files);
        }
    }
}

bool HasDocumentedFunction(const std::vector<const clang::FunctionDecl*>& functions,
                           clang::SourceManager& sourceManager)
{
    return std::any_of(functions.begin(), functions.end(), [&](const auto* function)
    {
        return IsDocumented(*function, sourceManager);
    });
}
}

std::string_view DocumentationRule::Id() const noexcept
{
    return "neowpf-documentation-coverage";
}

std::string_view DocumentationRule::Description() const noexcept
{
    return "Require documented public methods and documented functions in class-free headers.";
}

RuleSeverity DocumentationRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

void DocumentationRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(translationUnitDecl().bind("translation-unit"), &callback);
}

void DocumentationRule::OnMatch(
    const MatchFinder::MatchResult&, RuleContext& context)
{
    auto& sourceManager = context.SourceMgr();
    std::unordered_map<std::string, FileDeclarations> files;
    CollectDeclarations(*context.AST().getTranslationUnitDecl(), sourceManager, files);

    const auto severity = context.SettingsFor(Id()).severity;
    for (const auto& fileEntry : files)
    {
        const auto& file = fileEntry.second;
        for (const auto& classEntry : file.classes)
        {
            const auto& classInfo = classEntry.second;
            const auto* record = classInfo.definition;
            if (!record || !IsPublicClass(*record)) continue;

            bool hasDocumentation = classInfo.hasDocumentation;
            for (const auto* method : record->methods())
            {
                hasDocumentation |= IsDocumented(*method, sourceManager);
            }
            if (!hasDocumentation) continue;

            for (const auto* method : record->methods())
            {
                if (!IsDocumentableMethod(*method) ||
                    IsDocumented(*method, sourceManager)) continue;
                context.Report(Id(), method->getLocation(), severity,
                    method->getNameAsString() + " is undocumented in a documented class.");
            }
        }

        if (!file.hasClass && HasDocumentedFunction(file.functions, sourceManager))
        {
            for (const auto* function : file.functions)
            {
                if (IsDocumented(*function, sourceManager)) continue;
                context.Report(Id(), function->getLocation(), severity,
                    function->getNameAsString() + " is undocumented in a documented header.");
            }
        }
    }
}

std::string_view DocumentationTagRule::Id() const noexcept
{
    return "neowpf-documentation-tags";
}

std::string_view DocumentationTagRule::Description() const noexcept
{
    return "Reject @note and @todo tags in header files.";
}

RuleSeverity DocumentationTagRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

void DocumentationTagRule::Run(
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
const TreeRuleRegistrar<DocumentationTagRule> treeRegistrar;
}
}