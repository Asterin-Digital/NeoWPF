#pragma once

#include <NeoWPF/UI/Core/Visual.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace NeoWPF::Input::Core
{
    class MouseEventArgs;
    class KeyboardEventArgs;
}

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI {

class UIElement : public Visual {
public:
    bool IsVisible() const noexcept { return m_isVisible; }
    void SetVisibility(bool visible) noexcept { m_isVisible = visible; }

    bool IsEnabled() const noexcept { return m_isEnabled; }
    void SetEnabled(bool enabled) noexcept { m_isEnabled = enabled; }

    void InvalidateVisual() { m_isDirty = true; MarkTreeDirty(); }
    bool IsDirty() const noexcept { return m_isDirty; }
    void ClearDirty() noexcept {
        m_isDirty = false;
        ClearTreeDirty();
    }

    virtual void Measure(int availableWidth, int availableHeight) {}
    virtual void Arrange(int finalWidth, int finalHeight) {}

    virtual bool IsHitTestVisible() const noexcept { return true; }
    virtual bool IsInteractable() const noexcept { return true; }

    void SetTemplate(const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& templatePtr) {
        m_template = templatePtr;
        m_hasTemplateApplied = false;
    }

    const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& GetTemplate() const noexcept { return m_template; }

    bool HasTemplate() const noexcept { return static_cast<bool>(m_template); }

    bool ApplyTemplate() {
        if (!m_template || m_hasTemplateApplied) {
            return false;
        }

        m_template->Apply(this);
        m_hasTemplateApplied = true;
        return true;
    }

    void RegisterTemplateChild(const std::string& name, Visual* child) {
        if (name.empty() || !child) return;
        m_templateChildren[name] = child;
    }

    Visual* FindTemplateChild(const std::string& name) const {
        if (name.empty()) return nullptr;
        auto it = m_templateChildren.find(name);
        return it != m_templateChildren.end() ? it->second : nullptr;
    }

    bool HasTemplateChild(const std::string& name) const {
        if (name.empty()) return false;
        return m_templateChildren.find(name) != m_templateChildren.end();
    }

    void ClearTemplateChildren() noexcept { m_templateChildren.clear(); m_hasTemplateApplied = false; }

    virtual void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) {}
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) {}
    virtual void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) {}

private:
    bool m_isVisible = true;
    bool m_isEnabled = true;
    bool m_isDirty = true;
    bool m_hasTemplateApplied = false;
    std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase> m_template;
    std::unordered_map<std::string, Visual*> m_templateChildren;
};

}
