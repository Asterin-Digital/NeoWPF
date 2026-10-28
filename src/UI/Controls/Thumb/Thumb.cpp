#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
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

void Thumb::OnRoutedEvent(NeoWPF::Input::Core::RoutedEventArgs* args)
{
    if (!args) {
        return;
    }

    auto* pointerArgs = dynamic_cast<NeoWPF::Input::Core::RoutedPointerEventArgs*>(args);
    if (!pointerArgs) {
        return;
    }

    const auto* routedEvent = args->GetRoutedEvent();
    if (routedEvent == &NeoWPF::Input::Core::PointerPressedEvent()) {
        if (pointerArgs->GetChangedButton() != 0x01u || !IsInteractable()) {
            return;
        }

        if (!Input::InputManager::GetInstance().SetPointerCapture(
                pointerArgs->GetPointerId(), this)) {
            return;
        }
        if (auto parent = GetParentElement()) {
            parent->Focus();
        }

        const int x = static_cast<int>(pointerArgs->GetX());
        const int y = static_cast<int>(pointerArgs->GetY());
        m_isDragging = true;
        m_dragStartPointer = m_orientation == Orientation::Vertical ? y : x;
        DragStarted.Invoke(m_dragStartPointer);
        return;
    }

    auto& input = Input::InputManager::GetInstance();
    const bool ownsCapture = input.GetPointerCapture(pointerArgs->GetPointerId()) == this;
    if (!m_isDragging || (!ownsCapture &&
            routedEvent != &NeoWPF::Input::Core::PointerCanceledEvent())) {
        return;
    }

    const int x = static_cast<int>(pointerArgs->GetX());
    const int y = static_cast<int>(pointerArgs->GetY());
    if (routedEvent == &NeoWPF::Input::Core::PointerMovedEvent()) {
        const int pointer = m_orientation == Orientation::Vertical ? y : x;
        const int delta = pointer - m_dragStartPointer;
        m_dragStartPointer = pointer;
        DragDelta.Invoke(delta);
    } else if (routedEvent == &NeoWPF::Input::Core::PointerReleasedEvent() &&
               pointerArgs->GetChangedButton() == 0x01u) {
        m_isDragging = false;
        input.ReleasePointerCapture(pointerArgs->GetPointerId());
        DragCompleted.Invoke(m_orientation == Orientation::Vertical ? y : x);
    } else if (routedEvent == &NeoWPF::Input::Core::PointerCanceledEvent()) {
        m_isDragging = false;
        input.ReleasePointerCapture(pointerArgs->GetPointerId());
        DragCompleted.Invoke(m_dragStartPointer);
    }
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
