#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <cstdint>

namespace NeoWPF::UI {

class ScrollBar : public FrameworkElement {
public:
    ScrollBar();
    ~ScrollBar();

    void SetOrientation(bool vertical) noexcept { m_isVertical = vertical; InvalidateVisual(); }
    bool IsVertical() const noexcept { return m_isVertical; }

    void SetTrackColor(uint32_t color) noexcept { m_trackColor = color; InvalidateVisual(); }
    uint32_t GetTrackColor() const noexcept { return m_trackColor; }

    void SetThumbColor(uint32_t color) noexcept { m_thumbColor = color; InvalidateVisual(); }
    uint32_t GetThumbColor() const noexcept { return m_thumbColor; }

    void SetThumbSize(int size) noexcept { m_thumbSize = size < 1 ? 1 : size; InvalidateVisual(); }
    int GetThumbSize() const noexcept { return m_thumbSize; }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

private:
    bool m_isVertical = true;
    uint32_t m_trackColor = 0xFF2A2A2Au;
    uint32_t m_thumbColor = 0xFF5A5A5Au;
    int m_thumbSize = 24;
};

}
