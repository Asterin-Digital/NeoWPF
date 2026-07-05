#pragma once

#include <vector>
#include <memory>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>

namespace NeoWPF::UI {

class Visual : public std::enable_shared_from_this<Visual> {
public:
    Visual();
    virtual ~Visual();

    Visual* GetParent() const noexcept { return m_parent; }
    const std::vector<Visual*>& GetVisualChildren() const noexcept { return m_children; }
    std::vector<Visual*> GetVisualChildrenInZOrder() const;
    int GetZIndex() const noexcept { return m_zIndex; }
    void SetZIndex(int zIndex) noexcept {
        if (m_zIndex != zIndex) {
            m_zIndex = zIndex;
            MarkTreeDirty();
        }
    }
    static const std::vector<Visual*>& GetRegisteredVisuals();


    static void CaptureRootVisuals();

    void AddVisualChild(Visual* child);
    void AddChild(Visual* child);
    void RemoveVisualChild(Visual* child);

    bool IsTreeDirty() const noexcept { return m_isTreeDirty; }
    void MarkTreeDirty();

    void ClearTreeDirty() noexcept { m_isTreeDirty = false; }

    virtual void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) {}

    void DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options = NeoWPF::Diagnostics::VisualTreeDumpOptions{}) const;

private:
    static std::vector<Visual*>& GetRegistry();
    static std::vector<std::shared_ptr<Visual>>& GetRootVisualReferences();
    void KeepAliveAsRoot();

    Visual* m_parent = nullptr;
    std::vector<Visual*> m_children;
    int m_zIndex = 0;
    bool m_isTreeDirty = false;
};

}
