#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>

namespace NeoWPF::UI {

/*
    @summary
    Base class for all visual objects in the UI tree.

    @details
    Visual represents a node that can participate in layout, rendering, and tree traversal.
*/
class Visual : public std::enable_shared_from_this<Visual> {
public:
    static bool s_visualDebugging;

    Visual();

    virtual ~Visual();

    /*
        @summary
        Returns the parent of this visual.

        @internal
    */
    Visual* GetParent() const noexcept {
        std::lock_guard<std::recursive_mutex> lock(m_visualTreeMutex);
        return m_parent.lock().get();
    }

    /*
        @summary
        Returns the visual children attached to this node.

        @internal
    */
    std::vector<Visual*> GetVisualChildren() const;

    /*
        @summary
        Returns the logical children attached to this node.

        @internal
    */
    std::vector<Visual*> GetLogicalChildren() const;

    /*
        @summary
        Returns the visual children in drawing order.

        @internal
    */
    std::vector<Visual*> GetVisualChildrenInZOrder() const;

    /*
        @summary
        Returns the current z-index.
    */
    int GetZIndex() const noexcept { return m_zIndex; }

    /*
        @summary
        Sets the current z-index.
    */
    void SetZIndex(int zIndex) noexcept {
        if (m_zIndex != zIndex) {
            m_zIndex = zIndex;
            MarkTreeDirty();
        }
    }

    /*
        @summary
        Returns all registered visuals.

        @internal
    */
    static const std::vector<Visual*>& GetRegisteredVisuals();

    /*
        @summary
        Adds a visual child.

        @internal
    */
    void AddVisualChild(const std::shared_ptr<Visual>& child);

    /*
        @summary
        Adds a child visual.

        @internal
    */
    void AddChild(const std::shared_ptr<Visual>& child);

    /*
        @summary
        Adds a logical child.

        @internal
    */
    void AddLogicalChild(const std::shared_ptr<Visual>& child);

    /*
        @summary
        Removes a visual child.

        @internal
    */
    void RemoveVisualChild(const std::shared_ptr<Visual>& child);

    /*
        @summary
        Removes a visual child by raw pointer.

        @internal
    */
    void RemoveVisualChild(Visual* child);

    /*
        @summary
        Removes this visual from its current parent.

        @internal
    */
    void RemoveFromParent();

    /*
        @summary
        Returns whether the visual tree is dirty.

        @internal
    */
    bool IsTreeDirty() const noexcept { return m_isTreeDirty; }

    /*
        @summary
        Marks the visual tree as dirty.

        @internal
    */
    void MarkTreeDirty();

    /*
        @summary
        Clears the dirty flag.

        @internal
    */
    void ClearTreeDirty() noexcept { m_isTreeDirty = false; }

    /*
        @summary
        Builds render commands for this visual.

        @internal
    */
    virtual void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) {}

    /*
        @summary
        Dumps the visual tree for debugging.

        @internal
    */
    void DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options = NeoWPF::Diagnostics::VisualTreeDumpOptions{}) const;

private:
    /*
        @summary
        Returns the internal visual registry.

        @internal
    */
    static std::vector<Visual*>& GetRegistry();

    mutable std::recursive_mutex m_visualTreeMutex;
    std::weak_ptr<Visual> m_parent;
    std::vector<std::shared_ptr<Visual>> m_children;
    std::vector<std::shared_ptr<Visual>> m_logicalChildren;
    int m_zIndex = 0;
    bool m_isTreeDirty = false;
};

}
