#pragma once

#include <NeoLint/Core/IRule.h>
#include <NeoLint/Core/ITreeRule.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace neolint
{
using RuleFactory = std::function<std::unique_ptr<IRule>()>;
using TreeRuleFactory = std::function<std::unique_ptr<ITreeRule>()>;

class RuleRegistry
{
public:
    static RuleRegistry& Instance();
    void Register(std::string id, RuleFactory factory);
    void RegisterTree(std::string id, TreeRuleFactory factory);
    std::vector<std::string> Ids() const;
    std::unique_ptr<IRule> Create(std::string_view id) const;
    std::unique_ptr<ITreeRule> CreateTree(std::string_view id) const;
    bool Contains(std::string_view id) const;

private:
    std::vector<std::pair<std::string, RuleFactory>> rules_;
    std::vector<std::pair<std::string, TreeRuleFactory>> treeRules_;
};

template <typename RuleT>
class RuleRegistrar
{
public:
    RuleRegistrar()
    {
        RuleRegistry::Instance().Register(std::string(RuleT{}.Id()),
                                          [] { return std::make_unique<RuleT>(); });
    }
};

template <typename RuleT>
class TreeRuleRegistrar
{
public:
    TreeRuleRegistrar()
    {
        RuleRegistry::Instance().RegisterTree(std::string(RuleT{}.Id()),
                                              [] { return std::make_unique<RuleT>(); });
    }
};
}
