#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>
#include <memory>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

ScrollBar::ScrollBar()
    : FrameworkElement()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ScrollBarTemplate>());
    ApplyTemplate();
    SetWidth(16);
    SetHeight(120);
    SetClipToBounds(true);
}

ScrollBar::~ScrollBar() = default;

void ScrollBar::Measure(int availableWidth, int availableHeight)
{
    const bool isVertical = m_orientation == ScrollBarOrientation::Vertical;
    SetDesiredSize(isVertical ? 16 : 120, isVertical ? 120 : 16);
}

void ScrollBar::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    UpdateScrollState();
}

void ScrollBar::SetContentSize(int size) noexcept
{
    m_contentSize = std::max(0, size);
    if (m_value > GetScrollRange()) {
        SetValue(GetScrollRange());
    }
    InvalidateVisual();
}

void ScrollBar::SetViewportSize(int size) noexcept
{
    m_viewportSize = std::max(0, size);
    if (m_value > GetScrollRange()) {
        SetValue(GetScrollRange());
    }
    InvalidateVisual();
}

void ScrollBar::UpdateScrollTarget() noexcept
{
    if (!m_scrollTarget) {
        return;
    }

    if (m_orientation == ScrollBarOrientation::Vertical) {
        m_scrollTarget->SetRenderOffset(0, -m_value);
    } else {
        m_scrollTarget->SetRenderOffset(-m_value, 0);
    }
}

void ScrollBar::UpdateScrollState() noexcept
{
    if (!m_scrollTarget || !m_viewportElement) {
        return;
    }

    const int contentSize = m_orientation == ScrollBarOrientation::Vertical ? m_scrollTarget->GetDesiredHeight() : m_scrollTarget->GetDesiredWidth();
    const int viewportSize = m_orientation == ScrollBarOrientation::Vertical ? m_viewportElement->GetFinalHeight() : m_viewportElement->GetFinalWidth();
    SetContentSize(contentSize);
    SetViewportSize(viewportSize);
    UpdateScrollTarget();
}

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

int ScrollBar::GetScrollRange() const noexcept
{
    if (m_contentSize <= m_viewportSize) {
        return 0;
    }
    return m_contentSize - m_viewportSize;
}

int ScrollBar::GetThumbLength(int trackLength) const noexcept
{
    if (m_contentSize <= 0 || m_viewportSize <= 0) {
        return std::max(8, std::min(m_thumbSize, trackLength));
    }
    const int thumbLength = std::max(8, (m_viewportSize * trackLength) / m_contentSize);
    return std::min(thumbLength, trackLength);
}

int ScrollBar::GetThumbPosition(int trackLength, int thumbLength) const noexcept
{
    const int range = GetScrollRange();
    if (range <= 0 || trackLength <= thumbLength) {
        return 0;
    }
    return static_cast<int>((static_cast<long long>(m_value) * (trackLength - thumbLength)) / range);
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

void ScrollBar::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX, absY;
    GetAbsolutePosition(absX, absY);
    const auto clip = GetClipRect();

    auto* track = dynamic_cast<Templates::TemplateVisual*>(FindTemplateChild("PART_Track"));
    auto* thumb = dynamic_cast<Templates::TemplateVisual*>(FindTemplateChild("PART_Thumb"));

    if (track) {
        track->SetRect(absX, absY, GetFinalWidth(), GetFinalHeight());
        track->SetFillColor(m_trackColor);
        track->SetOpacity(GetOpacity());
        track->SetClipRect(clip);
        track->BuildRenderCommands(commands);
    } else {
        commands.DrawRect(static_cast<float>(absX), static_cast<float>(absY), static_cast<float>(GetFinalWidth()), static_cast<float>(GetFinalHeight()), m_trackColor, GetOpacity(), clip);
    }

    const bool isVertical = m_orientation == ScrollBarOrientation::Vertical;
    const int trackLength = isVertical ? GetFinalHeight() : GetFinalWidth();
    const int thumbLength = GetThumbLength(trackLength);
    const int thumbPosition = GetThumbPosition(trackLength, thumbLength);

    if (thumb) {
        if (isVertical) {
            thumb->SetRect(absX, absY + thumbPosition, GetFinalWidth(), thumbLength);
        } else {
            thumb->SetRect(absX + thumbPosition, absY, thumbLength, GetFinalHeight());
        }
        thumb->SetFillColor(m_thumbColor);
        thumb->SetOpacity(GetOpacity());
        thumb->SetClipRect(clip);
        thumb->BuildRenderCommands(commands);
    } else if (isVertical) {
        commands.DrawRect(static_cast<float>(absX), static_cast<float>(absY + thumbPosition), static_cast<float>(GetFinalWidth()), static_cast<float>(thumbLength), m_thumbColor, GetOpacity(), clip);
    } else {
        commands.DrawRect(static_cast<float>(absX + thumbPosition), static_cast<float>(absY), static_cast<float>(thumbLength), static_cast<float>(GetFinalHeight()), m_thumbColor, GetOpacity(), clip);
    }
}

}
