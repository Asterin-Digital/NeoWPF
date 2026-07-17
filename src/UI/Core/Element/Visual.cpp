#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/CallStackTracer.h>
#include <algorithm>
#include <memory>
#include <string>

// Base rule: Visual.cpp/h is not allowed to reach into UIElement.cpp/h. Same goes for 
// UIElement.cpp/h not being allowed to reach into FrameworkElement.cpp/h

// Please keep Visual.h file stable and don't modify it in any way unless you
// have a reasonable and good motivation to do so, modifying it will trigger a full recompile 

namespace NeoWPF::UI {

bool Visual::s_visualDebugging = false;

namespace {

bool ShouldWarnDanglingChild(const std::shared_ptr<Visual>& child)
{
    if (!child || NeoWPF::Diagnostics::IsDeveloperWarningSuppressionEnabled()) {
        return false;
    }

    return false;
}

}

void Visual::AddVisualChild(const std::shared_ptr<Visual>& child)
{
    if (!child) return;
    
    if (ShouldWarnDanglingChild(child)) {
        const std::wstring title = std::wstring(NeoWPF::Diagnostics::DeveloperWarnings::kDanglingChildReferenceCode) + L": dangling child reference detected";
        NeoWPF::Diagnostics::QueueDeveloperWarning(
            title,
            L"A visual child was attached without active shared ownership.",
            NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
    }

    child->m_parent = shared_from_this();
    m_children.push_back(child);
    if (!GetParent()) {
        KeepAliveAsRoot();
    }
    MarkTreeDirty();
}

void Visual::AddChild(const std::shared_ptr<Visual>& child)
{
    AddVisualChild(child);
    AddLogicalChild(child);
}

void Visual::RemoveVisualChild(const std::shared_ptr<Visual>& child)
{
    if (!child) return;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == child.get()) {
            m_children.erase(it);
            child->m_parent.reset();
            MarkTreeDirty();
            return;
        }
    }
}

std::vector<Visual*> Visual::GetVisualChildren() const
{
    std::vector<Visual*> children;
    children.reserve(m_children.size());
    for (const auto& child : m_children) {
        if (child) {
            children.push_back(child.get());
        }
    }
    return children;
}

std::vector<Visual*> Visual::GetVisualChildrenInZOrder() const
{
    std::vector<Visual*> orderedChildren;
    orderedChildren.reserve(m_children.size());
    for (const auto& child : m_children) {
        if (child) {
            orderedChildren.push_back(child.get());
        }
    }
    std::stable_sort(orderedChildren.begin(), orderedChildren.end(), [](Visual* left, Visual* right) {
        return left->GetZIndex() < right->GetZIndex();
    });
    return orderedChildren;
}

std::vector<Visual*> Visual::GetLogicalChildren() const
{
    std::vector<Visual*> children;
    children.reserve(m_logicalChildren.size());
    for (const auto& child : m_logicalChildren) {
        if (child) {
            children.push_back(child.get());
        }
    }
    return children;
}

void Visual::AddLogicalChild(const std::shared_ptr<Visual>& child)
{
    if (!child) return;
    m_logicalChildren.push_back(child);
}

void Visual::MarkTreeDirty()
{
    if (s_visualDebugging) {
        NeoWPF::Diagnostics::CallStackEntry stack[12];
        const int count = NeoWPF::Diagnostics::CaptureCallStack(stack, 12, 1);
        NeoWPF::Diagnostics::LogInfo("[Visual] MarkTreeDirty from %p (%s) [alreadyDirty=%s]", this, typeid(*this).name(), m_isTreeDirty ? "true" : "false");
        for (int i = 0; i < count; ++i) {
            if (stack[i].symbol[0] != '\0') {
                NeoWPF::Diagnostics::LogInfo("  caller[%d] %s @ %p", i, stack[i].symbol, stack[i].address);
            } else {
                NeoWPF::Diagnostics::LogInfo("  caller[%d] <unknown> @ %p", i, stack[i].address);
            }
        }
    }

    if (m_isTreeDirty) {
        return;
    }

    m_isTreeDirty = true;
    if (auto parent = m_parent.lock()) {
        parent->MarkTreeDirty();
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
            for (auto it = rootRefs.begin(); it != rootRefs.end(); ) {
                if (auto existing = it->lock()) {
                    if (existing.get() == this) {
                        return;
                    }
                    ++it;
                } else {
                    it = rootRefs.erase(it);
                }
            }
            rootRefs.emplace_back(self);
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

std::vector<std::weak_ptr<Visual>>& Visual::GetRootVisualReferences()
{
    static std::vector<std::weak_ptr<Visual>> rootReferences;
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
                rootRefs.emplace_back(visual->shared_from_this());
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

