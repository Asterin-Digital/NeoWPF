#pragma once

#include <NeoWPF/UI/Core/Element/Visual.h>
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

namespace NeoWPF::UI::Templates {
class ControlTemplateBase;
}

namespace NeoWPF::UI {

class UIElement : public Visual {
public:
    bool IsVisible() const noexcept { return m_isVisible; }
    void SetVisibility(bool visible) noexcept { m_isVisible = visible; }

    bool IsEnabled() const noexcept { return m_isEnabled; }
    void SetEnabled(bool enabled) noexcept { m_isEnabled = enabled; }

    float GetOpacity() const noexcept { return m_opacity; }
    void SetOpacity(float opacity) noexcept {
        if (opacity < 0.0f) {
            opacity = 0.0f;
        } else if (opacity > 1.0f) {
            opacity = 1.0f;
        }
        if (m_opacity != opacity) {
            m_opacity = opacity;
            InvalidateVisual();
        }
    }

    void InvalidateVisual();
    bool IsDirty() const noexcept { return m_isDirty; }
    void ClearDirty() noexcept;

    void BeginAnimation(float from, float to, float durationSeconds);

    virtual void Measure(int availableWidth, int availableHeight) {}
    virtual void Arrange(int finalWidth, int finalHeight) {}

    virtual bool IsHitTestVisible() const noexcept { return true; }
    virtual bool IsInteractable() const noexcept { return true; }

    void SetTemplate(const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& templatePtr);
    const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& GetTemplate() const noexcept { return m_template; }

    bool HasTemplate() const noexcept { return static_cast<bool>(m_template); }

    bool ApplyTemplate();
    void ClearTemplateVisualRoot() noexcept;

    void RegisterTemplateChild(const std::string& name, Visual* child);
    Visual* FindTemplateChild(const std::string& name) const;
    bool HasTemplateChild(const std::string& name) const;
    void ClearTemplateChildren() noexcept;

    virtual void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) {}
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) {}
    virtual void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) {}
    virtual void OnGotFocus() {}
    virtual void OnLostFocus() {}

protected:
    bool m_isEnabled = true;

private:
    bool m_isVisible = true;
    bool m_isDirty = true;
    float m_opacity = 1.0f;
    bool m_hasTemplateApplied = false;
    std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase> m_template;
    std::unordered_map<std::string, Visual*> m_templateChildren;
    Visual* m_appliedTemplateRoot = nullptr;
};

}
