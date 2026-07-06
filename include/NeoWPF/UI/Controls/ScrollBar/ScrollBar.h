#pragma once

#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <cstdint>

namespace NeoWPF::UI {

enum class ScrollBarOrientation {
    Vertical,
    Horizontal
};

class ScrollBarStyle;
void RegisterScrollBarStyles();

class ScrollBar : public FrameworkElement {
public:
    ScrollBar();
    ~ScrollBar();

    void SetOrientation(ScrollBarOrientation orientation) noexcept { m_orientation = orientation; InvalidateVisual(); }
    ScrollBarOrientation GetOrientation() const noexcept { return m_orientation; }

    void SetTrackColor(uint32_t color) noexcept { m_trackColor = color; InvalidateVisual(); }
    uint32_t GetTrackColor() const noexcept { return m_trackColor; }

    void SetThumbColor(uint32_t color) noexcept { m_thumbColor = color; InvalidateVisual(); }
    uint32_t GetThumbColor() const noexcept { return m_thumbColor; }

    void SetThumbSize(int size) noexcept { m_thumbSize = size < 1 ? 1 : size; InvalidateVisual(); }
    int GetThumbSize() const noexcept { return m_thumbSize; }

    void SetContentSize(int size) noexcept;
    void SetViewportSize(int size) noexcept;
    void SetScrollTarget(FrameworkElement* target) noexcept { m_scrollTarget = target; UpdateScrollState(); }
    void SetViewportElement(FrameworkElement* viewportElement) noexcept { m_viewportElement = viewportElement; UpdateScrollState(); }
    void SetValue(int value) noexcept;
    int GetValue() const noexcept { return m_value; }

    NeoWPF::Core::Event<int> ValueChanged;

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    void OnMouseDown(int x, int y, std::uint32_t buttonState) override;
    void OnMouseUp(int x, int y, std::uint32_t buttonState) override;
    void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) override;

private:
    int GetScrollRange() const noexcept;
    int GetThumbLength(int trackLength) const noexcept;
    int GetThumbPosition(int trackLength, int thumbLength) const noexcept;
    void UpdateValueFromPosition(int x, int y) noexcept;
    void UpdateScrollTarget() noexcept;
    void UpdateScrollState() noexcept;

    ScrollBarOrientation m_orientation = ScrollBarOrientation::Vertical;
    uint32_t m_trackColor = 0xFF2A2A2Au;
    uint32_t m_thumbColor = 0xFF5A5A5Au;
    int m_thumbSize = 24;
    int m_contentSize = 0;
    int m_viewportSize = 0;
    int m_value = 0;
    bool m_isDragging = false;
    int m_dragStartValue = 0;
    int m_dragStartPointer = 0;
    FrameworkElement* m_scrollTarget = nullptr;
    FrameworkElement* m_viewportElement = nullptr;
};

}
