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

std::vector<std::string> RuleRegistry::Ids() const
{
    std::vector<std::string> ids;
    ids.reserve(rules_.size());
    for (const auto& [id, factory] : rules_)
    {
        ids.push_back(id);
    }
    return ids;
}

std::unique_ptr<IRule> RuleRegistry::Create(std::string_view id) const
{
    const auto match = std::find_if(rules_.begin(), rules_.end(),
        [id](const auto& entry) { return entry.first == id; });
    return match == rules_.end() ? nullptr : match->second();
}
}
