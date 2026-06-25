#include <NeoWPF/UI/Core/Visual.h>

namespace NeoWPF::UI {

Visual::Visual()
{
    GetRegistry().push_back(this);
}

Visual::~Visual()
{
    auto& registry = GetRegistry();
    for (auto it = registry.begin(); it != registry.end(); ++it) {
        if (*it == this) {
            registry.erase(it);
            break;
        }
    }
}

std::vector<Visual*>& Visual::GetRegistry()
{
    static std::vector<Visual*> registry;
    return registry;
}

const std::vector<Visual*>& Visual::GetRegisteredVisuals()
{
    return GetRegistry();
}

}
