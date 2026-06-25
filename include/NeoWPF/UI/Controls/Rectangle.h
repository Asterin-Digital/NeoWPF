#pragma once

#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <cstdint>

namespace NeoWPF::UI {

class Rectangle : public FrameworkElement {
public:
    Rectangle() noexcept = default;

    void SetFillColor(uint32_t color) noexcept { m_fillColor = color; InvalidateVisual(); }
    uint32_t GetFillColor() const noexcept { return m_fillColor; }

    void SetWidth(int width) noexcept { m_width = width; InvalidateVisual(); }
    void SetHeight(int height) noexcept { m_height = height; InvalidateVisual(); }
    void SetSize(int width, int height) noexcept { m_width = width; m_height = height; InvalidateVisual(); }

    int GetWidth() const noexcept { return m_width; }
    int GetHeight() const noexcept { return m_height; }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

private:
    uint32_t m_fillColor = 0xFF808080u;
    int m_width = 0;
    int m_height = 0;
};

}
