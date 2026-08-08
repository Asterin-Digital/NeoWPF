#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <algorithm>

namespace NeoWPF::UI {

void ScrollBar::SetValue(int value) noexcept
{
    RangeBase::SetValue(static_cast<double>(std::clamp(value, 0, GetScrollRange())));
}

void ScrollBar::LineUp() noexcept
{
    SetValue(GetValue() - std::max(1, static_cast<int>(GetSmallChange())));
}

void ScrollBar::LineDown() noexcept
{
    SetValue(GetValue() + std::max(1, static_cast<int>(GetSmallChange())));
}

void ScrollBar::PageUp() noexcept
{
    SetValue(GetValue() - std::max(1, static_cast<int>(GetLargeChange())));
}

void ScrollBar::PageDown() noexcept
{
    SetValue(GetValue() + std::max(1, static_cast<int>(GetLargeChange())));
}

void ScrollBar::UpdateValueFromPosition(int x, int y) noexcept
{
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const bool isVertical = m_orientation == Orientation::Vertical;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int effectiveTrackLength = std::max(0, trackLength);
    const int thumbLength = GetThumbLength(effectiveTrackLength);
    const int availableTrack = std::max(0, effectiveTrackLength - thumbLength);

    int pointerPos = isVertical ? y - absY : x - absX;
    pointerPos = std::clamp(pointerPos - thumbLength / 2, 0, availableTrack);

    const int range = GetScrollRange();
    if (range <= 0 || availableTrack <= 0) {
        SetValue(0);
        return;
    }

    const int newValue = static_cast<int>((static_cast<long long>(pointerPos) * range + availableTrack / 2) / availableTrack);
    NeoWPF::Diagnostics::LogInfo("[ScrollBar] UpdateValueFromPosition: pointerPos=%d track=%d thumb=%d range=%d newValue=%d", pointerPos, trackLength, thumbLength, range, newValue);
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

    const bool isVertical = m_orientation == Orientation::Vertical;
    const int localPos = isVertical ? y - absY : x - absX;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int effectiveTrackLength = std::max(0, trackLength);
    const int thumbLength = GetThumbLength(effectiveTrackLength);
    const int thumbPosition = GetThumbPosition(effectiveTrackLength, thumbLength);

    if (localPos >= thumbPosition && localPos < thumbPosition + thumbLength) {
        m_isDragging = true;
        if (m_thumb) {
            m_thumb->SetDragging(true);
        }
        Input::InputManager::GetInstance().SetMouseCapture(this);
        m_dragStartValue = GetValue();
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
    if (m_thumb) {
        m_thumb->SetDragging(false);
    }
    Input::InputManager::GetInstance().SetMouseCapture(nullptr);
}

void ScrollBar::OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (!m_isDragging) {
        return;
    }

    if (!args->IsLeftButtonDown()) {
        m_isDragging = false;
        if (m_thumb) {
            m_thumb->SetDragging(false);
        }
        Input::InputManager::GetInstance().SetMouseCapture(nullptr);
        return;
    }

    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const bool isVertical = m_orientation == Orientation::Vertical;
    const int pointerPos = isVertical ? args->GetY() - absY : args->GetX() - absX;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int effectiveTrackLength = std::max(0, trackLength);
    const int thumbLength = GetThumbLength(effectiveTrackLength);
    const int availableTrack = std::max(0, effectiveTrackLength - thumbLength);

    const int dragDelta = pointerPos - m_dragStartPointer;
    const int range = GetScrollRange();
    if (range <= 0 || availableTrack <= 0) {
        return;
    }

    const int valueDelta = static_cast<int>((static_cast<long long>(dragDelta) * range + availableTrack / 2) / availableTrack);
    SetValue(static_cast<int>(std::clamp(m_dragStartValue + valueDelta, 0.0, static_cast<double>(range))));
}

} // namespace NeoWPF::UI
