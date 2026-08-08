#pragma once

#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <cstdint>

namespace NeoWPF::UI {

class Thumb : public Control {
public:
    Thumb() = default;
    ~Thumb() override = default;

    void SetOrientation(Orientation orientation) noexcept { m_orientation = orientation; }
    Orientation GetOrientation() const noexcept { return m_orientation; }

    bool IsDragging() const noexcept { return m_isDragging; }
    void SetDragging(bool dragging) noexcept { m_isDragging = dragging; }
    void SetBounds(int x, int y, int width, int height) noexcept { m_x = x; m_y = y; m_width = width; m_height = height; }
    void SetVisible(bool visible) noexcept { m_isVisible = visible; }

    void OnMouseDown(int x, int y, std::uint32_t buttonState) override;
    void OnMouseUp(int x, int y, std::uint32_t buttonState) override;
    void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    NeoWPF::Core::Event<int> DragStarted;
    NeoWPF::Core::Event<int> DragDelta;
    NeoWPF::Core::Event<int> DragCompleted;

private:
    Orientation m_orientation = Orientation::Vertical;
    bool m_isDragging = false;
    bool m_isVisible = true;
    int m_dragStartPointer = 0;
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
};

}
