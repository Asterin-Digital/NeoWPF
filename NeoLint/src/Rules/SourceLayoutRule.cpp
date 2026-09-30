#include <NeoLint/Rules/SourceLayoutRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/ASTMatchers/ASTMatchers.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <string>
#include <string_view>

namespace neolint
{
using namespace clang::ast_matchers;

namespace
{
constexpr std::array kHeaderExtensions{".h"};
constexpr std::array kSourceExtensions{".cpp"};

std::string Lowercase(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

bool HasExtension(const std::filesystem::path& path, const auto& extensions)
{
    const auto extension = Lowercase(path.extension().string());
    return std::ranges::find(extensions, extension) != extensions.end();
}

bool HasDirectory(const std::filesystem::path& path, std::string_view directory)
{
    const auto expected = Lowercase(std::string(directory));
    for (const auto& component : path) {
        if (Lowercase(component.string()) == expected) return true;
    }
    return false;
}
}

std::string_view SourceLayoutRule::Id() const noexcept
{
    return "neowpf-source-layout";
}

std::string_view SourceLayoutRule::Description() const noexcept
{
    return "Reject header files under src and source files under include.";
}

RuleSeverity SourceLayoutRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Error;
}

void SourceLayoutRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(translationUnitDecl().bind("source-layout-translation-unit"), &callback);
}

void SourceLayoutRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    if (!result.Context || !result.SourceManager) return;

    const auto location = result.SourceManager->getLocForStartOfFile(
        result.SourceManager->getMainFileID());
    const std::filesystem::path filePath(
        result.SourceManager->getFilename(location).str());
    const bool underSource = HasDirectory(filePath, "src");
    const bool underInclude = HasDirectory(filePath, "include");
    const bool isHeader = HasExtension(filePath, kHeaderExtensions);
    const bool isSource = HasExtension(filePath, kSourceExtensions);

    if (underSource && isHeader) {
        context.Report(Id(), location, context.SettingsFor(Id()).severity,
                       "Header file is not allowed under src/: " + filePath.string());
    } else if (underInclude && isSource) {
        context.Report(Id(), location, context.SettingsFor(Id()).severity,
                       "Source file is not allowed under include/: " + filePath.string());
    }
}

namespace
{
const RuleRegistrar<SourceLayoutRule> registrar;
}
}