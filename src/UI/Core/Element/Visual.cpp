#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <algorithm>
#include <memory>
#include <string>

namespace NeoWPF::UI {
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
    
    if (auto* presenter = dynamic_cast<Content::ContentPresenter*>(child.get())) {
        if (auto* parent = dynamic_cast<FrameworkElement*>(this)) {
            if (!GetParent() && parent->GetFinalWidth() == 0 && parent->GetFinalHeight() == 0) {
                const std::wstring title = L"EarlyPresenterAttachment: ContentPresenter added during initialization";
                NeoWPF::Diagnostics::QueueDeveloperWarning(
                    title,
                    L"ContentPresenter was added to ContentControl during initialization (constructor). Defer attachment until after layout begins.",
                    NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
                return;
            }
        }
    }

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

