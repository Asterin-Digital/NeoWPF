#pragma once

#include <NeoWPF/UI/Core/Visual.h>

namespace NeoWPF::UI {

class UIElement : public Visual {
public:
    bool IsVisible() const noexcept { return m_isVisible; }
    void SetVisibility(bool visible) noexcept { m_isVisible = visible; }

    void InvalidateVisual() { m_isDirty = true; MarkTreeDirty(); }
    bool IsDirty() const noexcept { return m_isDirty; }
    void ClearDirty() noexcept {
        m_isDirty = false;
        ClearTreeDirty();
    }

    virtual void Measure(int availableWidth, int availableHeight) {}
    virtual void Arrange(int finalWidth, int finalHeight) {}

private:
    bool m_isVisible = true;
    bool m_isDirty = true;
};

}
