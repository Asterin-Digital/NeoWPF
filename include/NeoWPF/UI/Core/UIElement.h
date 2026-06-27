#pragma once

#include <NeoWPF/UI/Core/Visual.h>

namespace NeoWPF::Input::Core
{
    class MouseEventArgs;
    class KeyboardEventArgs;
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
};

}
