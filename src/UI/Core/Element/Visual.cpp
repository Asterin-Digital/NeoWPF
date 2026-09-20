#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Core/Threading/ThreadAffinity.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/CallStackTracer.h>
#include <algorithm>
#include <memory>
#include <string>
#include <thread>
#include <functional>
#include <mutex>

namespace NeoWPF::UI {


namespace {

bool CanRunVisualLifecycle() noexcept
{
    return !NeoWPF::Core::Threading::ThreadAffinity::IsMainThreadRegistered() ||
        NeoWPF::Core::Threading::ThreadAffinity::IsMainThread();
}

void DebugBreakPortable() noexcept
{
#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#endif
}

bool ShouldWarnDanglingChild(const std::shared_ptr<Visual>& child)
{
    if (!child || NeoWPF::Diagnostics::IsDeveloperWarningSuppressionEnabled()) {
        return false;
    }

    return false;
}

bool ContainsVisualChild(const std::vector<std::shared_ptr<Visual>>& children, Visual* child)
{
    if (!child) {
        return false;
    }

    for (const auto& existing : children) {
        if (existing.get() == child) {
            return true;
        }
    }

    return false;
}

bool AttachChildToParent(Visual* parent,
                         const std::shared_ptr<Visual>& child,
                         const char* logPrefix)
{
    if (!parent || !child) {
        return false;
    }

    if (auto currentParent = child->GetParent()) {
        if (currentParent == parent) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Debug,
                "%s Duplicate child detected. parent=%p child=%p",
                logPrefix,
                parent,
                child.get());
            DebugBreakPortable();
            return false;
        }

        currentParent->RemoveVisualChild(child);
    }

    child->SetParent(parent);
    return true;
}

}

void Visual::AddVisualChild(const std::shared_ptr<Visual>& child)
{
    if (!child) return;

    // [tree-mtx]
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Debug,
        "[Visual::AddVisualChild] thread=%zu this=%p child=%p children=%zu capacity=%zu",
        std::hash<std::thread::id>{}(std::this_thread::get_id()),
        this,
        child.get(),
        m_children.size(),
        m_children.capacity());

    if (ShouldWarnDanglingChild(child)) {
        const std::wstring title = std::wstring(NeoWPF::Diagnostics::DeveloperWarnings::kDanglingChildReferenceCode) + L": dangling child reference detected";
        NeoWPF::Diagnostics::QueueDeveloperWarning(
            title,
            L"A visual child was attached without active shared ownership.",
            NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
    }

    if (!AttachChildToParent(this, child, "[Visual::AddVisualChild]")) {
        return;
    }

    if (CanRunVisualLifecycle() && IsInitialized() && !child->IsInitialized() && !child->IsInitializing()) {
        child->Initialize();
    }

    if (ContainsVisualChild(m_children, child.get())) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Debug,
            "[Visual::AddVisualChild] Duplicate child detected after detach. parent=%p child=%p",
            this,
            child.get());

        DebugBreakPortable();
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
        "[Visual::AddVisualChild] Before push_back");

    m_children.push_back(child);

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
        "[Visual::AddVisualChild] After push_back");

    if (CanRunVisualLifecycle() && IsInitialized()) {
        MarkLayoutDirty();
    } // @note not sure if this will cause any issues (previosuly it double marked layout dirty unconditionally)
     // so if anything breaks, just do it unconditionally in here, but unless it causes problems keep it
}


// note: internal controls are not allowed to call this method, it's meant to be apart of the public API
// NOT for internal usage whatsoever 
void Visual::AddChild(const std::shared_ptr<Visual>& child)
{
    if (!child) return;

    // [tree-mtx]
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);

    if (!AttachChildToParent(this, child, "[Visual::AddChild]")) {
        return;
    }

    if (ContainsVisualChild(m_children, child.get())) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Debug,
            "[Visual::AddChild] Duplicate child detected after detach. parent=%p child=%p",
            this,
            child.get());
        return;
    }

    m_children.push_back(child);

    if (CanRunVisualLifecycle() && IsInitialized()) {
        MarkLayoutDirty();
    }

    if (CanRunVisualLifecycle() && IsInitialized() && !child->IsInitialized() && !child->IsInitializing()) {
        child->Initialize();
    }

    if (ContainsVisualChild(m_logicalChildren, child.get())) {
        if (CanRunVisualLifecycle()) {
            MarkLayoutDirty();
        }
        return;
    }

    m_logicalChildren.push_back(child);

    if (CanRunVisualLifecycle()) {
        MarkLayoutDirty();
        OnAddChild(child.get());
    }
}

void Visual::RemoveVisualChild(const std::shared_ptr<Visual>& child)
{
    if (!child) return;
    RemoveVisualChild(child.get());
}

void Visual::RemoveVisualChild(Visual* child)
{
    if (!child) return;

    // [tree-mtx]
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == child) {
            m_children.erase(it);
            break;
        }
    }

    for (auto it = m_logicalChildren.begin(); it != m_logicalChildren.end(); ++it) {
        if (it->get() == child) {
            m_logicalChildren.erase(it);
            break;
        }
    }

    child->SetParent(nullptr);
    if (CanRunVisualLifecycle()) {
        OnRemoveVisualChild(child);
        MarkLayoutDirty();
    }
}

void Visual::RemoveFromParent()
{
    if (m_parent) {
        m_parent->RemoveVisualChild(this);
    }
}

std::vector<Visual*> Visual::GetVisualChildren() const
{
    // [tree-mtx]
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);

    std::vector<Visual*> children;
    children.reserve(m_children.size());
    for (const auto& child : m_children) {
        if (child) {
            children.push_back(child.get());
        }
    }
    return children;
}

std::vector<Visual*> Visual::GetVisualDescendants() const
{
    std::vector<Visual*> descendants;
    const auto visit = [&descendants](Visual* current, const auto& visit) -> void {
        if (!current) {
            return;
        }
        descendants.push_back(current);
        const auto children = current->GetVisualChildren();
        for (Visual* child : children) {
            visit(child, visit);
        }
    };

    for (Visual* child : GetVisualChildren()) {
        visit(child, visit);
    }
    return descendants;
}

std::vector<Visual*> Visual::GetVisualChildrenInZOrder() const
{
    // [tree-mtx]
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);

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
    // [tree-mtx]
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);

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

    // [tree-mtx]
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);
    m_logicalChildren.push_back(child);
}

void Visual::MarkTreeDirty()
{
    if (!CanRunVisualLifecycle() || m_isTreeDirty) {
        return;
    }

    m_isTreeDirty = true;
    if (m_parent) {
        m_parent->MarkTreeDirty();
    }
}

void Visual::MarkLayoutDirty()
{
    if (!CanRunVisualLifecycle() || m_isLayoutDirty) {
        return;
    }

    m_isLayoutDirty = true;
    MarkTreeDirty();
    if (m_parent) {
        m_parent->MarkLayoutDirty();
    }
}

void Visual::Initialize()
{
    if (!CanRunVisualLifecycle() || m_isInitialized || m_initializationInProgress) {
        return;
    }

    m_initializationInProgress = true;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info,
        "[Visual::Initialize] start this=%p parent=%p children=%zu",
        this,
        m_parent,
        m_children.size());

    try {
        OnInitialize();

        std::vector<std::shared_ptr<Visual>> childrenSnapshot;
        {
            // [tree-mtx]
            std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);
            childrenSnapshot = m_children;
        }

        for (const auto& child : childrenSnapshot) {
            if (child) {
                child->Initialize();
            }
        }

        m_isInitialized = true;
        m_initializationInProgress = false;
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info,
            "[Visual::Initialize] complete this=%p parent=%p children=%zu",
            this,
            m_parent,
            m_children.size());
    } catch (const std::exception& ex) {
        m_initializationInProgress = false;
        NeoWPF::Diagnostics::LogError(
            "[Visual::Initialize] failed this=%p parent=%p children=%zu error=%s",
            this,
            m_parent,
            m_children.size(),
            ex.what());
        throw;
    } catch (...) {
        m_initializationInProgress = false;
        NeoWPF::Diagnostics::LogError(
            "[Visual::Initialize] failed this=%p parent=%p children=%zu error=<unknown>",
            this,
            m_parent,
            m_children.size());
        throw;
    }
}

Visual::Visual()
{
    GetRegistry().push_back(this);
}

Visual* Visual::GetParent() const noexcept
{
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);
    return m_parent;
}

void Visual::SetParent(Visual* parent) noexcept
{
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);
    m_parent = parent;
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

std::vector<Visual*>& Visual::GetRegistry() // not thread safe, used only by Visual Tree Dumper which is thread safe
{
    static std::vector<Visual*> registry;
    return registry;
}

const std::vector<Visual*>& Visual::GetRegisteredVisuals()
{
    return GetRegistry();
}

void Visual::DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options) const
{
    NeoWPF::Diagnostics::VisualTreeDumper::DumpTree(const_cast<Visual*>(this), options);
}

Visual* Visual::FindInVisualTree(std::function<bool(Visual*)> predicate) noexcept
{
    if (!predicate) {
        return nullptr;
    }

    // [tree-mtx]
    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);

    try {
        if (predicate(this)) {
            return this;
        }
    } catch (...) {
        return nullptr;
    }

    auto children = GetVisualChildren();
    for (auto* child : children) {
        if (!child) continue;
        auto found = child->FindInVisualTree(predicate);
        if (found) {
            return found;
        }
    }

    return nullptr;
}

}

