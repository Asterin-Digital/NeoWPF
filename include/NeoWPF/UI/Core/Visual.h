#pragma once

#include <vector>
#include <memory>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::UI {

class Visual {
public:
    virtual ~Visual() = default;

    Visual* GetParent() const noexcept { return m_parent; }
    const std::vector<Visual*>& GetVisualChildren() const noexcept { return m_children; }

    void AddVisualChild(Visual* child) {
        if (!child) return;
        child->m_parent = this;
        m_children.push_back(child);
    }

    void RemoveVisualChild(Visual* child) {
        if (!child) return;
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if (*it == child) {
                m_children.erase(it);
                child->m_parent = nullptr;
                return;
            }
        }
    }

    virtual void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) {}

private:
    Visual* m_parent = nullptr;
    std::vector<Visual*> m_children;
};

}
