#include <NeoLint/Rules/VirtualDestructorRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/AST/DeclCXX.h>
#include <clang/ASTMatchers/ASTMatchers.h>

namespace neolint
{
using namespace clang::ast_matchers;

std::string_view VirtualDestructorRule::Id() const noexcept
{
    return "neowpf-virtual-destructor";
}

std::string_view VirtualDestructorRule::Description() const noexcept
{
    return "Require an accessible virtual destructor for polymorphic base classes.";
}

RuleSeverity VirtualDestructorRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

void VirtualDestructorRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(cxxRecordDecl(isDefinition())
        .bind("polymorphic-class-without-virtual-destructor"), &callback);
}

void VirtualDestructorRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    const auto* record = result.Nodes.getNodeAs<clang::CXXRecordDecl>(
        "polymorphic-class-without-virtual-destructor");
    if (!record || !record->isPolymorphic()) return;
    const auto* destructor = record->getDestructor();
    if (!destructor || destructor->isVirtual() ||
        destructor->getAccess() != clang::AS_public) return;

    context.Report(Id(), record->getLocation(), context.SettingsFor(Id()).severity,
        "Polymorphic class '" + record->getQualifiedNameAsString() +
        "' has a public non-virtual destructor; deleting it through a base pointer is undefined");
}

namespace
{
const RuleRegistrar<VirtualDestructorRule> registrar;
}
}