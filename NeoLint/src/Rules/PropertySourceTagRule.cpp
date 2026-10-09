#include <NeoLint/Rules/PropertySourceTagRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/ASTMatchers/ASTMatchers.h>

#include <algorithm>
#include <ranges>

namespace clang::ast_matchers
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100)
#endif
AST_MATCHER_P(FunctionDecl, hasConfiguredPropertyCallName, std::string, Name)
{
    return Node.getNameAsString() == Name;
}

AST_MATCHER_P(CXXMethodDecl, hasConfiguredPropertyWrapperName,
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

std::string_view PropertySourceTagRule::Id() const noexcept
{
    return "neowpf-property-source-tag";
}

std::string_view PropertySourceTagRule::Description() const noexcept
{
    return "Require trigger and style property wrappers to preserve their value source.";
}

RuleSeverity PropertySourceTagRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

void PropertySourceTagRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(
        callExpr(callee(functionDecl(hasConfiguredPropertyCallName(checkedCallName_))
                            .bind("checked-call")),
            hasAncestor(cxxMethodDecl(hasConfiguredPropertyWrapperName(wrapperMethodNames_))
                            .bind("wrapper-method")))
            .bind("invalid-wrapper-source"), &callback);
}

bool PropertySourceTagRule::ValidateOptions(
    const RuleOptions& options, std::string& error) const
{
    for (const auto& key : options.Keys())
    {
        const bool valid =
            (key == "wrapper_method_names" &&
                options.TypeOf(key) == RuleOptions::Type::StringList) ||
            ((key == "checked_call_name" || key == "forbidden_enumerator") &&
                options.TypeOf(key) == RuleOptions::Type::String) ||
            (key == "argument_index" &&
                options.TypeOf(key) == RuleOptions::Type::Integer &&
                options.GetInteger(key) >= 0);
        if (valid) continue;
        error = "rule '" + std::string(Id()) + "' has invalid option '" + key + "'";
        return false;
    }
    return true;
}

void PropertySourceTagRule::Configure(const RuleOptions& options)
{
    wrapperMethodNames_ = options.GetStrings("wrapper_method_names", wrapperMethodNames_);
    checkedCallName_ = options.GetString("checked_call_name", checkedCallName_);
    forbiddenEnumerator_ = options.GetString("forbidden_enumerator", forbiddenEnumerator_);
    argumentIndex_ = static_cast<std::size_t>(
        options.GetInteger("argument_index", static_cast<std::int64_t>(argumentIndex_)));
}

void PropertySourceTagRule::OnMatch(const MatchFinder::MatchResult& result,
                                    RuleContext& context)
{
    const auto* call = result.Nodes.getNodeAs<clang::CallExpr>("invalid-wrapper-source");
    if (!call) return;
    const auto* calledFunction = result.Nodes.getNodeAs<clang::FunctionDecl>("checked-call");
    const auto* wrapperMethod = result.Nodes.getNodeAs<clang::CXXMethodDecl>("wrapper-method");
    if (!calledFunction || !wrapperMethod ||
        calledFunction->getNameAsString() != checkedCallName_ ||
        std::ranges::find(wrapperMethodNames_, wrapperMethod->getNameAsString()) ==
            wrapperMethodNames_.end() || argumentIndex_ >= call->getNumArgs()) return;
    const auto* argument = call->getArg(static_cast<unsigned>(argumentIndex_))
        ->IgnoreParenImpCasts();
    const auto* reference = llvm::dyn_cast<clang::DeclRefExpr>(argument);
    const auto* enumerator = reference ?
        llvm::dyn_cast<clang::EnumConstantDecl>(reference->getDecl()) : nullptr;
    if (!enumerator || enumerator->getNameAsString() != forbiddenEnumerator_) return;
    context.Report(Id(), call->getExprLoc(), context.SettingsFor(Id()).severity,
                   "SetTriggerPropertyValue and SetStylePropertyValue must not pass "
                   "PropertyValueSource::Local");
}

namespace
{
const RuleRegistrar<PropertySourceTagRule> registrar;
}
}
