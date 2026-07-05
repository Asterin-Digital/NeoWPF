#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <algorithm>

namespace NeoWPF::UI {

void Visual::AddVisualChild(Visual* child)
{
    if (!child) return;
    child->m_parent = this;
    m_children.push_back(child);
    if (!m_parent) {
        KeepAliveAsRoot();
    }
    MarkTreeDirty();
}

void Visual::AddChild(Visual* child)
{
    AddVisualChild(child);
}

void Visual::RemoveVisualChild(Visual* child)
{
    if (!child) return;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (*it == child) {
            m_children.erase(it);
            child->m_parent = nullptr;
            MarkTreeDirty();
            return;
        }
    }
}

std::vector<Visual*> Visual::GetVisualChildrenInZOrder() const
{
    std::vector<Visual*> orderedChildren = m_children;
    std::stable_sort(orderedChildren.begin(), orderedChildren.end(), [](Visual* left, Visual* right) {
        return left->GetZIndex() < right->GetZIndex();
    });
    return orderedChildren;
}

void Visual::MarkTreeDirty()
{
    if (m_isTreeDirty) {
        return;
    }
    m_isTreeDirty = true;
    if (m_parent) {
        m_parent->MarkTreeDirty();
    }
}

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

void Visual::DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options) const
{
    NeoWPF::Diagnostics::VisualTreeDumper::DumpTree(const_cast<Visual*>(this), options);
}

}

