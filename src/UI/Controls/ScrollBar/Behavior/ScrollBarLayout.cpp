#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace NeoWPF::UI {

void ScrollBar::SetContentSize(int size) noexcept
{
    m_contentSize = std::max(0, size);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug, "[ScrollBar] SetContentSize: size=%d range=%d value=%d", m_contentSize, GetScrollRange(), GetValue());
    if (GetValue() > GetScrollRange()) {
        SetValue(GetScrollRange());
    }
    InvalidateVisual();
}

void ScrollBar::SetViewportSize(int size) noexcept
{
    m_viewportSize = std::max(0, size);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug, "[ScrollBar] SetViewportSize: size=%d range=%d value=%d", m_viewportSize, GetScrollRange(), GetValue());
    if (GetValue() > GetScrollRange()) {
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
            ? (m_orientation == Orientation::Vertical ? m_scrollTarget->GetDesiredHeight() : m_scrollTarget->GetDesiredWidth())
            : m_contentSize;
        const int viewportSize = hasViewportElement
            ? (m_orientation == Orientation::Vertical ? m_viewportElement->GetFinalHeight() : m_viewportElement->GetFinalWidth())
            : m_viewportSize;

        SetContentSize(contentSize);
        SetViewportSize(viewportSize);
    }

    SetMinimum(0.0);
    SetMaximum(static_cast<double>(GetScrollRange()));
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
    const int effectiveTrackLength = std::max(0, trackLength);
    if (effectiveTrackLength <= 0) {
        return 0;
    }

    int thumbLength = effectiveTrackLength;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug, "[ScrollBar] GetThumbLength: start track=%d content=%d viewport=%d thumbSize=%d range=%d value=%d", effectiveTrackLength, m_contentSize, m_viewportSize, m_thumbSize, GetScrollRange(), GetValue());
    if (m_contentSize > 0 && m_viewportSize > 0 && m_contentSize > m_viewportSize) {
        const int denominator = std::max(1, m_contentSize);
        const int proportionalThumb = std::max(1, (m_viewportSize * effectiveTrackLength) / denominator);
        thumbLength = std::clamp(proportionalThumb, std::min(m_thumbSize, effectiveTrackLength), effectiveTrackLength);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug, "[ScrollBar] GetThumbLength: proportional track=%d content=%d viewport=%d proportionalThumb=%d", effectiveTrackLength, m_contentSize, m_viewportSize, thumbLength);
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug, "[ScrollBar] GetThumbLength: final track=%d content=%d viewport=%d thumb=%d range=%d value=%d", effectiveTrackLength, m_contentSize, m_viewportSize, thumbLength, GetScrollRange(), GetValue());
    return thumbLength;
}

int ScrollBar::GetThumbPosition(int trackLength, int thumbLength) const noexcept
{
    const int range = GetScrollRange();
    const int effectiveTrackLength = std::max(0, trackLength);
    int thumbPosition = 0;
    if (range <= 0 || effectiveTrackLength <= thumbLength) {
        thumbPosition = 0;
    }
    else {
        thumbPosition = static_cast<int>((static_cast<long long>(GetValue()) * (effectiveTrackLength - thumbLength)) / range);
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug, "[ScrollBar] GetThumbPosition: track=%d thumb=%d value=%d range=%d position=%d", trackLength, thumbLength, GetValue(), range, thumbPosition);
    return thumbPosition;
}

} // namespace NeoWPF::UI
