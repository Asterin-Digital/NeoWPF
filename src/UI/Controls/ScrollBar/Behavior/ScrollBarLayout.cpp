#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace NeoWPF::UI {

bool ScrollBar::s_scrollBarDebug = true;

void ScrollBar::SetContentSize(int size) noexcept
{
    m_contentSize = std::max(0, size);
    if (s_scrollBarDebug) {
        NeoWPF::Diagnostics::LogInfo("[ScrollBar] SetContentSize: size=%d range=%d value=%d", m_contentSize, GetScrollRange(), m_value);
    }
    if (m_value > GetScrollRange()) {
        SetValue(GetScrollRange());
    }
    InvalidateVisual();
}

void ScrollBar::SetViewportSize(int size) noexcept
{
    m_viewportSize = std::max(0, size);
    if (s_scrollBarDebug) {
        NeoWPF::Diagnostics::LogInfo("[ScrollBar] SetViewportSize: size=%d range=%d value=%d", m_viewportSize, GetScrollRange(), m_value);
    }
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
    const int clampedTrackLength = std::max(0, trackLength);
    int thumbLength = 0;
    if (m_contentSize <= 0 || m_viewportSize <= 0 || m_contentSize <= m_viewportSize) {
        thumbLength = std::max(8, std::min(m_thumbSize, clampedTrackLength));
    }
    else {
        thumbLength = std::max(8, (m_viewportSize * clampedTrackLength) / m_contentSize);
        thumbLength = std::min(thumbLength, clampedTrackLength);
    }

    if (s_scrollBarDebug) {
        NeoWPF::Diagnostics::LogInfo("[ScrollBar] GetThumbLength: track=%d content=%d viewport=%d thumb=%d range=%d value=%d", clampedTrackLength, m_contentSize, m_viewportSize, thumbLength, GetScrollRange(), m_value);
    }
    return thumbLength;
}

int ScrollBar::GetThumbPosition(int trackLength, int thumbLength) const noexcept
{
    const int range = GetScrollRange();
    int thumbPosition = 0;
    if (range <= 0 || trackLength <= thumbLength) {
        thumbPosition = 0;
    }
    else {
        thumbPosition = static_cast<int>((static_cast<long long>(m_value) * (trackLength - thumbLength)) / range);
    }

    if (s_scrollBarDebug) {
        NeoWPF::Diagnostics::LogInfo("[ScrollBar] GetThumbPosition: track=%d thumb=%d value=%d range=%d position=%d", trackLength, thumbLength, m_value, range, thumbPosition);
    }
    return thumbPosition;
}

} // namespace NeoWPF::UI
