#include <NeoWPF/UI/Core/Visual.h>

namespace NeoWPF::UI {

Visual::Visual()
{
    GetRegistry().push_back(this);
}

void Visual::KeepAliveAsRoot()
{
    auto& rootRefs = GetRootVisualReferences();
    if (!GetParent()) {
        try {
            auto self = shared_from_this();
            for (auto& existing : rootRefs) {
                if (existing.get() == this) {
                    return;
                }
            }
            rootRefs.push_back(self);
        } catch (const std::bad_weak_ptr&) {
        }
    }
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

std::vector<std::shared_ptr<Visual>>& Visual::GetRootVisualReferences()
{
    static std::vector<std::shared_ptr<Visual>> rootReferences;
    return rootReferences;
}

const std::vector<Visual*>& Visual::GetRegisteredVisuals()
{
    return GetRegistry();
}

void Visual::CaptureRootVisuals()
{
    auto& rootRefs = GetRootVisualReferences();
    rootRefs.clear();

    for (Visual* visual : GetRegistry()) {
        if (!visual->GetParent()) {
            try {
                rootRefs.push_back(visual->shared_from_this());
            } catch (const std::bad_weak_ptr&) {
            }
        }
    }
}

}

