#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <algorithm>

namespace NeoWPF::UI {

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

void ScrollBar::UpdateScrollState() noexcept
{
    const bool hasScrollTarget = m_scrollTarget != nullptr;
    const bool hasViewportElement = m_viewportElement != nullptr;

    if (hasScrollTarget || hasViewportElement || m_contentSize > 0 || m_viewportSize > 0) {
        const int contentSize = hasScrollTarget
            ? (m_orientation == ScrollBarOrientation::Vertical ? m_scrollTarget->GetDesiredHeight() : m_scrollTarget->GetDesiredWidth())
            : m_contentSize;
        const int viewportSize = hasViewportElement
            ? (m_orientation == ScrollBarOrientation::Vertical ? m_viewportElement->GetFinalHeight() : m_viewportElement->GetFinalWidth())
            : m_viewportSize;

        SetContentSize(contentSize);
        SetViewportSize(viewportSize);
    }

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

} // namespace NeoWPF::UI
