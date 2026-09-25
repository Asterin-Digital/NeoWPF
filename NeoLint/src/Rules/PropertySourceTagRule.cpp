#include <NeoLint/Rules/PropertySourceTagRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/ASTMatchers/ASTMatchers.h>

namespace neolint
{
using namespace clang::ast_matchers;

std::string_view PropertySourceTagRule::Id() const noexcept
{
    return "neowpf-property-source-tag";
}

std::string_view PropertySourceTagRule::Description() const noexcept
{
    return "Reject Local value sources inside Setter and Trigger-family code.";
}

RuleSeverity PropertySourceTagRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

void PropertySourceTagRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(
        callExpr(
            callee(functionDecl(hasName("SetValueInternal"))),
            hasArgument(2, declRefExpr(to(enumConstantDecl(hasName("Local"))))),
            hasAncestor(cxxMethodDecl(ofClass(cxxRecordDecl(matchesName("(Setter|Trigger|Style)")))))
        ).bind("invalid-set-value-source"), &callback);
}

void PropertySourceTagRule::OnMatch(const MatchFinder::MatchResult& result,
                                    RuleContext& context)
{
    const auto* call = result.Nodes.getNodeAs<clang::CallExpr>("invalid-set-value-source");
    if (!call) return;
    context.Report(Id(), call->getExprLoc(), context.Config().SettingsFor(Id()).severity,
                   "Setter, Trigger, and Style code must not record PropertyValueSource::Local");
}

namespace
{
const RuleRegistrar<PropertySourceTagRule> registrar;
}
}
