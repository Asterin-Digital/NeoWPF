#include <NeoWPF/UI/Core/Element/Visual.h>
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

    if (auto currentParent = child->GetParent()) {
        if (currentParent == this) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Debug,
                "[Visual::AddVisualChild] Duplicate child detected. parent=%p child=%p",
                this,
                child.get());
            __debugbreak();
            return;
        }

        currentParent->RemoveVisualChild(child);
    }

    child->m_parent = this;

    if (IsInitialized() && !child->IsInitialized() && !child->IsInitializing()) {
        child->Initialize();
    }

    for (const auto& existing : m_children)
    {
        if (existing.get() == child.get()) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Debug,
                "[Visual::AddVisualChild] Duplicate child detected after detach. parent=%p child=%p",
                this,
                child.get());

            __debugbreak();
        }
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
        "[Visual::AddVisualChild] Before push_back");

    m_children.push_back(child);

    if (IsInitialized()) {
        MarkTreeDirty();
    }
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
        "[Visual::AddVisualChild] After push_back");

    MarkTreeDirty();
}


// note: internal controls are not allowed to call this method, it's meant to be apart of the public API
// NOT for internal usage whatsoever 
void Visual::AddChild(const std::shared_ptr<Visual>& child)
{
    if (!child) return;

    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);

    if (auto currentParent = child->GetParent()) {
        if (currentParent == this) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Debug,
                "[Visual::AddChild] Duplicate child detected. parent=%p child=%p",
                this,
                child.get());
            return;
        }

        currentParent->RemoveVisualChild(child);
    }

    child->m_parent = this;

    for (const auto& existing : m_children)
    {
        if (existing.get() == child.get()) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Debug,
                "[Visual::AddChild] Duplicate child detected after detach. parent=%p child=%p",
                this,
                child.get());
            return;
        }
    }

    m_children.push_back(child);

    if (IsInitialized()) {
        MarkTreeDirty();
    }

    if (IsInitialized() && !child->IsInitialized() && !child->IsInitializing()) {
        child->Initialize();
    }

    for (const auto& existing : m_logicalChildren)
    {
        if (existing.get() == child.get())
        {
            MarkTreeDirty();
            return;
        }
    }

    m_logicalChildren.push_back(child);

    MarkTreeDirty();
}

void Visual::RemoveVisualChild(const std::shared_ptr<Visual>& child)
{
    if (!child) return;
    RemoveVisualChild(child.get());
}

void Visual::RemoveVisualChild(Visual* child)
{
    if (!child) return;

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

    child->m_parent = nullptr;
    OnRemoveVisualChild(child);
    MarkTreeDirty();
}

void Visual::RemoveFromParent()
{
    if (m_parent) {
        m_parent->RemoveVisualChild(this);
    }
}

std::vector<Visual*> Visual::GetVisualChildren() const
{
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

std::vector<Visual*> Visual::GetVisualChildrenInZOrder() const
{
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

    std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);
    m_logicalChildren.push_back(child);
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

void Visual::Initialize()
{
    if (m_isInitialized || m_initializationInProgress) {
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

void Visual::DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options) const
{
    NeoWPF::Diagnostics::VisualTreeDumper::DumpTree(const_cast<Visual*>(this), options);
}

Visual* Visual::FindInVisualTree(std::function<bool(Visual*)> predicate) noexcept
{
    if (!predicate) {
        return nullptr;
    }

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

