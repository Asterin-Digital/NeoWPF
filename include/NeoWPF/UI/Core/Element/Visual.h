/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <functional>
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

    Visual();

    virtual ~Visual();

    /*
        @summary
        Initializes this visual and its visual children.

        @internal
    */
    virtual void Initialize();

    /*
        @summary
        Returns whether this visual has completed initialization.

        @internal
    */
    bool IsInitialized() const noexcept { return m_isInitialized; }

    /*
        @summary
        Returns whether initialization is currently in progress.

        @internal
    */
    bool IsInitializing() const noexcept { return m_initializationInProgress; }

    /*
        @summary
        Returns the parent of this visual.

        @internal
    */
    Visual* GetParent() const noexcept;

    /*
        @summary
        Sets the parent of this visual.

        @param parent
        The new parent to assign, or nullptr to detach this visual from its current parent.

        @internal
    */
    void SetParent(Visual* parent) noexcept;

    /*
        @summary
        Returns the visual children attached to this node.

        @internal
    */
    std::vector<Visual*> GetVisualChildren() const;

    std::vector<Visual*> GetVisualDescendants() const;

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
        Called after a child visual has been removed.

        @internal
    */
    virtual void OnRemoveVisualChild(Visual* child) {}

    virtual void OnAddChild(Visual* child) {}

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

    bool IsLayoutDirty() const noexcept { return m_isLayoutDirty; }

    /*
        @summary
        Marks the visual tree as dirty.

        @internal
    */
    void MarkTreeDirty();

    void MarkLayoutDirty();

    /*
        @summary
        Clears the dirty flag.

        @internal
    */
    void ClearTreeDirty() noexcept { m_isTreeDirty = false; }

    void ClearLayoutDirty() noexcept { m_isLayoutDirty = false; }

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

    /*
        @summary
        Searches the visual tree for the first element matching the predicate.

        @details
        Performs a depth-first search of visual children. Returns the first element
        for which the predicate returns true, or nullptr if no match is found.

        @param predicate
        A function that returns true if the visual matches the search criteria.

        @return
        Raw pointer to the matching visual, or nullptr if not found.

        @internal
    */
    Visual* FindInVisualTree(std::function<bool(Visual*)> predicate) noexcept;

protected:
    /*
        @summary
        Extension point for derived classes to perform initialization work.

        @internal
    */
    virtual void OnInitialize() {}

private:
    /*
        @summary
        Returns the internal visual registry.

        @internal
    */
    static std::vector<Visual*>& GetRegistry();

    mutable std::recursive_mutex m_visualTreeMutex;
    Visual* m_parent = nullptr;
    std::vector<std::shared_ptr<Visual>> m_children;
    std::vector<std::shared_ptr<Visual>> m_logicalChildren;
    int m_zIndex = 0;
    bool m_isTreeDirty = false;
    bool m_isLayoutDirty = false;
    bool m_isInitialized = false;
    bool m_initializationInProgress = false;
};

}
