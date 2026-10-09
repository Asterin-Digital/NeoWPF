#include <NeoLint/Rules/DPRegistrationLocationRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/ASTMatchers/ASTMatchers.h>

#include <algorithm>
#include <ranges>
#include <string>

namespace clang::ast_matchers
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100)
#endif
AST_MATCHER_P(FunctionDecl, hasConfiguredRegistrationName,
              std::vector<std::string>, Names)
{
    return std::ranges::find(Names, Node.getNameAsString()) != Names.end();
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

namespace neolint
{
using namespace clang::ast_matchers;

std::string_view DPRegistrationLocationRule::Id() const noexcept
{
    return "neowpf-dp-registration-location";
}

std::string_view DPRegistrationLocationRule::Description() const noexcept
{
    return "Keep dependency-property registration in dedicated *Properties.cpp files.";
}

RuleSeverity DPRegistrationLocationRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

void DPRegistrationLocationRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(callExpr(callee(functionDecl(
        hasConfiguredRegistrationName(registrationNames_))))
        .bind("dependency-property-registration"), &callback);
}

bool DPRegistrationLocationRule::ValidateOptions(
    const RuleOptions& options, std::string& error) const
{
    for (const auto& key : options.Keys())
    {
        const bool valid =
            (key == "allowed_file_suffixes" &&
                options.TypeOf(key) == RuleOptions::Type::StringList) ||
            (key == "registration_function_names" &&
                options.TypeOf(key) == RuleOptions::Type::StringList);
        if (valid) continue;
        error = "rule '" + std::string(Id()) + "' has invalid option '" + key + "'";
        return false;
    }
    return true;
}

void DPRegistrationLocationRule::Configure(const RuleOptions& options)
{
    allowedSuffixes_ = options.GetStrings("allowed_file_suffixes", allowedSuffixes_);
    registrationNames_ = options.GetStrings("registration_function_names", registrationNames_);
}

void DPRegistrationLocationRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    const auto* call = result.Nodes.getNodeAs<clang::CallExpr>(
        "dependency-property-registration");
    if (!call) return;
    const auto* function = call->getDirectCallee();
    if (!function || std::ranges::find(registrationNames_, function->getNameAsString()) ==
        registrationNames_.end()) return;
    if (function->getName() == "Register" || function->getName() == "RegisterAttached")
    {
        const auto* method = llvm::dyn_cast<clang::CXXMethodDecl>(function);
        if (!method || method->getParent()->getName() != "DependencyProperty") return;
    }

    const auto presumed = context.SourceMgr().getPresumedLoc(call->getExprLoc());
    if (presumed.isInvalid()) return;

    const std::string filename = presumed.getFilename();
    if (std::ranges::any_of(allowedSuffixes_, [&](const std::string& suffix) {
        return filename.ends_with(suffix);
    }))
    {
        return;
    }

    context.Report(Id(), call->getExprLoc(), context.SettingsFor(Id()).severity,
                   "Dependency-property registration must be in a *Properties.cpp file");
}

namespace
{
const RuleRegistrar<DPRegistrationLocationRule> registrar;
}
}
