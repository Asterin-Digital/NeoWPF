#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace NeoWPF::UI {

bool Thumb::IsInteractable() const noexcept
{
    const double width = GetFinalWidth() > m_width ? GetFinalWidth() : m_width;
    const double height = GetFinalHeight() > m_height ? GetFinalHeight() : m_height;
    const bool interactable = m_isVisible && width > 0.0 && height > 0.0;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Info,
        "[Thumb] IsInteractable: visible=%s width=%g height=%g result=%s",
        m_isVisible ? "true" : "false",
        width,
        height,
        interactable ? "true" : "false");
    return interactable;
}

void Thumb::SetBounds(int x, int y, int width, int height) noexcept
{
    const bool positionChanged = m_x != x || m_y != y;
    const bool sizeChanged = m_width != width || m_height != height;

    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;

    if (positionChanged) {
        SetLayoutOffset(x, y);
    }

    SetDesiredSize(width, height);
    FrameworkElement::Arrange(width, height);

    if (positionChanged || sizeChanged) {
        InvalidateVisual();
    }
}

void Thumb::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    if ((buttonState & 0x01) == 0) {
        return;
    }

    if (!IsInteractable()) {
        return;
    }

    if (auto parent = GetParentElement()) {
        parent->Focus();
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Info,
        "[Thumb] OnMouseDown: x=%d y=%d buttonState=0x%X",
        x,
        y,
        buttonState);

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
    if (!m_isVisible) {
        return;
    }

    const double width = GetFinalWidth() > m_width ? GetFinalWidth() : m_width;
    const double height = GetFinalHeight() > m_height ? GetFinalHeight() : m_height;
    if (width <= 0.0 || height <= 0.0) {
        return;
    }

    const auto renderTransform = GetWorldRenderTransform();
    double absX = 0.0;
    double absY = 0.0;
    GetAbsolutePosition(absX, absY);

    const auto clip = GetClipRect();
    const auto cornerRadius = GetCornerRadius();
    const auto backgroundBrush = std::dynamic_pointer_cast<SolidColorBrush>(GetBackgroundBrush());
    if (!backgroundBrush) {
        return;
    }

    commands.DrawRoundedRect(
        absX,
        absY,
        width,
        height,
        cornerRadius.TopLeft,
        cornerRadius.TopRight,
        cornerRadius.BottomRight,
        cornerRadius.BottomLeft,
        backgroundBrush->GetArgb(),
        static_cast<float>(GetOpacity() * backgroundBrush->GetOpacity()),
        clip,
        renderTransform.rotationRadians,
        renderTransform.pivot.X,
        renderTransform.pivot.Y);
}

}
