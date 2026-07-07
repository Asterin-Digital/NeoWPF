#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <algorithm>

namespace NeoWPF::UI {

void ScrollBar::SetValue(int value) noexcept
{
    const int newValue = std::clamp(value, 0, GetScrollRange());
    if (newValue == m_value) {
        return;
    }
    m_value = newValue;
    InvalidateVisual();
    ValueChanged.Invoke(m_value);
    UpdateScrollTarget();
}

void ScrollBar::UpdateValueFromPosition(int x, int y) noexcept
{
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const bool isVertical = m_orientation == ScrollBarOrientation::Vertical;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int thumbLength = GetThumbLength(trackLength);
    const int availableTrack = std::max(0, trackLength - thumbLength);

    int pointerPos = isVertical ? y - absY : x - absX;
    pointerPos = std::clamp(pointerPos - thumbLength / 2, 0, availableTrack);

    const int range = GetScrollRange();
    if (range <= 0 || availableTrack <= 0) {
        SetValue(0);
        return;
    }

    const int newValue = static_cast<int>((static_cast<long long>(pointerPos) * range + availableTrack / 2) / availableTrack);
    SetValue(newValue);
}

void ScrollBar::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    if ((buttonState & 0x01) == 0) {
        return;
    }

    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const bool isVertical = m_orientation == ScrollBarOrientation::Vertical;
    const int localPos = isVertical ? y - absY : x - absX;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int thumbLength = GetThumbLength(trackLength);
    const int thumbPosition = GetThumbPosition(trackLength, thumbLength);

    if (localPos >= thumbPosition && localPos < thumbPosition + thumbLength) {
        m_isDragging = true;
        Input::InputManager::GetInstance().SetMouseCapture(this);
        m_dragStartValue = m_value;
        m_dragStartPointer = localPos;
        return;
    }

    UpdateValueFromPosition(x, y);
}

void ScrollBar::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    if (!m_isDragging) {
        return;
    }

    m_isDragging = false;
    Input::InputManager::GetInstance().SetMouseCapture(nullptr);
}

void ScrollBar::OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (!m_isDragging) {
        return;
    }

    if (!args->IsLeftButtonDown()) {
        m_isDragging = false;
        Input::InputManager::GetInstance().SetMouseCapture(nullptr);
        return;
    }

    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const bool isVertical = m_orientation == ScrollBarOrientation::Vertical;
    const int pointerPos = isVertical ? args->GetY() - absY : args->GetX() - absX;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int thumbLength = GetThumbLength(trackLength);
    const int availableTrack = std::max(0, trackLength - thumbLength);

    const int dragDelta = pointerPos - m_dragStartPointer;
    const int range = GetScrollRange();
    if (range <= 0 || availableTrack <= 0) {
        return;
    }

    const int valueDelta = static_cast<int>((static_cast<long long>(dragDelta) * range + availableTrack / 2) / availableTrack);
    SetValue(std::clamp(m_dragStartValue + valueDelta, 0, range));
}

} // namespace NeoWPF::UI
