#pragma once

#include <NeoLint/Core/IRule.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace neolint
{
using RuleFactory = std::function<std::unique_ptr<IRule>()>;

class RuleRegistry
{
public:
    static RuleRegistry& Instance();
    void Register(std::string id, RuleFactory factory);
    std::vector<std::string> Ids() const;
    std::unique_ptr<IRule> Create(std::string_view id) const;

private:
    std::vector<std::pair<std::string, RuleFactory>> rules_;
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
}
