#include <NeoLint/Core/RuleRegistry.h>

#include <algorithm>

namespace neolint
{
RuleRegistry& RuleRegistry::Instance()
{
    static RuleRegistry registry;
    return registry;
}

void RuleRegistry::Register(std::string id, RuleFactory factory)
{
    const auto existing = std::find_if(rules_.begin(), rules_.end(),
        [&id](const auto& entry) { return entry.first == id; });
    if (existing == rules_.end())
    {
        rules_.emplace_back(std::move(id), std::move(factory));
    }
}

void RuleRegistry::RegisterTree(std::string id, TreeRuleFactory factory)
{
    const auto existing = std::find_if(treeRules_.begin(), treeRules_.end(),
        [&id](const auto& entry) { return entry.first == id; });
    if (existing == treeRules_.end())
    {
        treeRules_.emplace_back(std::move(id), std::move(factory));
    }
}

std::vector<std::string> RuleRegistry::Ids() const
{
    std::vector<std::string> ids;
    ids.reserve(rules_.size() + treeRules_.size());
    for (const auto& entry : rules_)
    {
        ids.push_back(entry.first);
    }
    for (const auto& entry : treeRules_)
    {
        ids.push_back(entry.first);
    }
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    return ids;
}

std::unique_ptr<IRule> RuleRegistry::Create(std::string_view id) const
{
    const auto match = std::find_if(rules_.begin(), rules_.end(),
        [id](const auto& entry) { return entry.first == id; });
    return match == rules_.end() ? nullptr : match->second();
}

std::unique_ptr<ITreeRule> RuleRegistry::CreateTree(std::string_view id) const
{
    const auto match = std::find_if(treeRules_.begin(), treeRules_.end(),
        [id](const auto& entry) { return entry.first == id; });
    return match == treeRules_.end() ? nullptr : match->second();
}

bool RuleRegistry::Contains(std::string_view id) const
{
    return std::any_of(rules_.begin(), rules_.end(),
        [id](const auto& entry) { return entry.first == id; }) ||
        std::any_of(treeRules_.begin(), treeRules_.end(),
        [id](const auto& entry) { return entry.first == id; });
}
}
