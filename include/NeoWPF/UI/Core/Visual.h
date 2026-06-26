#pragma once

#include <vector>
#include <memory>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::UI {

class Visual : public std::enable_shared_from_this<Visual> {
public:
    Visual();
    virtual ~Visual();

    Visual* GetParent() const noexcept { return m_parent; }
    const std::vector<Visual*>& GetVisualChildren() const noexcept { return m_children; }
    static const std::vector<Visual*>& GetRegisteredVisuals();


    static void CaptureRootVisuals();

    void AddVisualChild(Visual* child) {
        if (!child) return;
        child->m_parent = this;
        m_children.push_back(child);
        if (!m_parent) {
            KeepAliveAsRoot();
        }
        MarkTreeDirty();
    }

    void RemoveVisualChild(Visual* child) {
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

    bool IsTreeDirty() const noexcept { return m_isTreeDirty; }
    void MarkTreeDirty() {
        if (m_isTreeDirty) {
            return;
        }
        m_isTreeDirty = true;
        if (m_parent) {
            m_parent->MarkTreeDirty();
        }
    }

    void ClearTreeDirty() noexcept { m_isTreeDirty = false; }

    virtual void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) {}

private:
    static std::vector<Visual*>& GetRegistry();
    static std::vector<std::shared_ptr<Visual>>& GetRootVisualReferences();
    void KeepAliveAsRoot();

    Visual* m_parent = nullptr;
    std::vector<Visual*> m_children;
    bool m_isTreeDirty = false;
};

}
