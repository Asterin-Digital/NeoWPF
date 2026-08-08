#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/Events.h>
#include <algorithm>

namespace NeoWPF::UI {

void Thumb::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    if ((buttonState & 0x01) == 0) {
        return;
    }

    m_isDragging = true;
    m_dragStartPointer = m_orientation == Orientation::Vertical ? y : x;
    Input::InputManager::GetInstance().SetMouseCapture(this);
    DragStarted.Invoke(m_dragStartPointer);
}

void Thumb::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    if (!m_isDragging) {
        return;
    }

    m_isDragging = false;
    Input::InputManager::GetInstance().SetMouseCapture(nullptr);
    DragCompleted.Invoke(m_orientation == Orientation::Vertical ? y : x);
}

void Thumb::OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (!m_isDragging || !args) {
        return;
    }

    const int pointer = m_orientation == Orientation::Vertical ? args->GetY() : args->GetX();
    const int delta = pointer - m_dragStartPointer;
    m_dragStartPointer = pointer;
    DragDelta.Invoke(delta);
}

void Thumb::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!m_isVisible || m_width <= 0 || m_height <= 0) {
        return;
    }

    const auto clip = GetClipRect();
    const auto cornerRadius = GetCornerRadius();
    commands.DrawRoundedRect(
        static_cast<float>(m_x),
        static_cast<float>(m_y),
        static_cast<float>(m_width),
        static_cast<float>(m_height),
        cornerRadius.TopLeft,
        cornerRadius.TopRight,
        cornerRadius.BottomRight,
        cornerRadius.BottomLeft,
        GetBackgroundColor().ToArgb(),
        GetOpacity(),
        clip);
}

}
