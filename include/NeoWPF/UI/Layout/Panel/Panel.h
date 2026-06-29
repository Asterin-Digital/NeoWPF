#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::Input::Core
{
    class MouseEventArgs;
}

namespace NeoWPF::UI {

class Panel : public FrameworkElement {
public:
    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) override {}
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) override {}

    void SetBackgroundColor(uint32_t color) noexcept { m_backgroundColor = color; InvalidateVisual(); }
    uint32_t GetBackgroundColor() const noexcept { return m_backgroundColor; }

private:
    uint32_t m_backgroundColor = 0x00000000u;
};

}
