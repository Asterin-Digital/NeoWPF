#include <NeoWPF/UI/Controls/ItemsControl/Virtualization/ItemsControlVirtualization.h>
#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <vector>
#include <memory>

namespace NeoWPF::UI::Controls::ItemsControl::Virtualization {

ItemsControlVirtualizer::ItemsControlVirtualizer(NeoWPF::UI::ItemsControl* owner, int itemHeight) noexcept
    : m_owner(owner)
    , m_itemHeight(std::max(1, itemHeight))
{
    m_estimatedItemHeight = m_itemHeight;
}

int ItemsControlVirtualizer::GetExtentWidth() const noexcept
{
    return m_owner ? m_owner->GetWidth() : 0;
}

int ItemsControlVirtualizer::GetExtentHeight() const noexcept
{
    if (!m_owner) return 0;
    const int count = m_owner->GetItemCount();
    int measuredTotal = m_bit.Total();
    int unmeasured = std::max(0, count - m_measuredCount);
    int total = measuredTotal + unmeasured * m_estimatedItemHeight;
    int result = total + std::max(0, m_owner->GetPadding() * 2);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
        "[Virtualizer.GetExtentHeight] count=%d measured=%d estHeight=%d measuredTotal=%d unmeasured=%d total=%d",
        count, m_measuredCount, m_estimatedItemHeight, measuredTotal, unmeasured, result);
    return result;
}

int ItemsControlVirtualizer::GetViewportWidth() const noexcept { return m_viewportWidth; }
int ItemsControlVirtualizer::GetViewportHeight() const noexcept { return m_viewportHeight; }

int ItemsControlVirtualizer::GetHorizontalOffset() const noexcept { return m_horizontalOffset; }
int ItemsControlVirtualizer::GetVerticalOffset() const noexcept {
    if (m_logicalScrolling) return GetOffsetForIndex(m_verticalIndexOffset);
    return m_verticalOffset;
}

void ItemsControlVirtualizer::SetHorizontalOffset(int offset) noexcept
{
    m_horizontalOffset = std::max(0, offset);
}

void ItemsControlVirtualizer::SetVerticalOffset(int offset) noexcept
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
        "[Virtualizer] SetVerticalOffset requested=%d logical=%d", offset, m_logicalScrolling);

    if (m_logicalScrolling) {
        const int count = m_owner ? m_owner->GetItemCount() : 0;
        const int clampedIndex = std::clamp(offset, 0, std::max(0, count - 1));
        if (clampedIndex == m_verticalIndexOffset) return;
        m_verticalIndexOffset = clampedIndex;
        RecomputeRealizedRange();
        if (m_owner) m_owner->InvalidateVisual();
        return;
    }

    const int extent = GetExtentHeight();
    const int maxOffset = std::max(0, extent - m_viewportHeight);
    const int clamped = std::clamp(offset, 0, maxOffset);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
        "[Virtualizer] SetVerticalOffset clamped=%d max=%d (extent=%d - viewport=%d) current=%d", clamped, maxOffset, extent, m_viewportHeight, m_verticalOffset);
    if (clamped == m_verticalOffset) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Trace,
            "[Virtualizer] SetVerticalOffset early return: clamped==current");
        return;
    }
    m_verticalOffset = clamped;
    RecomputeRealizedRange();
    if (m_owner) m_owner->InvalidateVisual();
}

void ItemsControlVirtualizer::LineLeft() noexcept {}
void ItemsControlVirtualizer::LineRight() noexcept {}
void ItemsControlVirtualizer::LineUp() noexcept { SetVerticalOffset(m_verticalOffset - m_itemHeight); }
void ItemsControlVirtualizer::LineDown() noexcept { SetVerticalOffset(m_verticalOffset + m_itemHeight); }

void ItemsControlVirtualizer::PageLeft() noexcept {}
void ItemsControlVirtualizer::PageRight() noexcept {}
void ItemsControlVirtualizer::PageUp() noexcept { SetVerticalOffset(m_verticalOffset - m_viewportHeight); }
void ItemsControlVirtualizer::PageDown() noexcept { SetVerticalOffset(m_verticalOffset + m_viewportHeight); }

void ItemsControlVirtualizer::MouseWheel(int delta) noexcept
{
    SetVerticalOffset(m_verticalOffset + delta);
}

void ItemsControlVirtualizer::SetViewportSize(int width, int height) noexcept
{
    m_viewportWidth = std::max(0, width);
    m_viewportHeight = std::max(0, height);
    RecomputeRealizedRange();
}

void ItemsControlVirtualizer::RecomputeRealizedRange() noexcept
{
    if (!m_owner) return;
    const int count = m_owner->GetItemCount();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
        "[Virtualizer] RecomputeRealizedRange count=%d viewport=%dx%d vOffset=%d logical=%d measured=%d estHeight=%d",
        count, m_viewportWidth, m_viewportHeight, m_verticalOffset, m_logicalScrolling, m_measuredCount, m_estimatedItemHeight);
    if (count == 0) {
        m_firstIndex = 0;
        m_lastIndex = -1;
        return;
    }

    if (static_cast<int>(m_measuredHeights.size()) != count) {
        m_measuredHeights.assign(count, -1);
        m_bit.Init(count);
        m_measuredCount = 0;
        m_countBit.Init(count);
        m_sampleSum = 0;
        m_sampleCount = 0;
        m_estimatedItemHeight = m_itemHeight;
    }

    if (m_logicalScrolling) {
        const int buffer = 3;
        int first = std::clamp(m_verticalIndexOffset, 0, count - 1);
        int last = first;
        int usedHeight = 0;
        auto gen = m_owner->GetGenerator();
        const auto& realItems = m_owner->GetItems();
        const auto& src = m_owner->GetItemsSource();
        while (usedHeight < m_viewportHeight && last < count) {
            if (m_measuredHeights[last] < 0) {
                std::shared_ptr<NeoWPF::UI::FrameworkElement> container;
                if (last < static_cast<int>(realItems.size())) container = realItems[last];
                else if (gen && last < static_cast<int>(src.size())) container = gen->Acquire(src[last], last, m_owner->GetItemTemplate());
                if (container) {
                    container->Measure(m_viewportWidth, 0);
                    UpdateMeasuredHeight(last, std::max(1, container->GetDesiredHeight()));
                    if (last >= static_cast<int>(realItems.size()) && gen) gen->Recycle(last, container);
                }
            }
            usedHeight += GetMeasuredHeight(last);
            ++last;
        }
        m_firstIndex = std::clamp(first - buffer, 0, count - 1);
        m_lastIndex = std::clamp(last + buffer - 1, 0, count - 1);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Trace,
            "[Virtualizer] logical realize first=%d last=%d usedHeight=%d", m_firstIndex, m_lastIndex, usedHeight);
        return;
    }

    const int padding = m_owner->GetPadding();
    const int target = std::max(0, m_verticalOffset - padding);

    int measuredTotal = m_bit.Total();
    int first = 0;
        if (measuredTotal > target) {

            first = m_bit.LowerBound(target);
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Trace,
                "[Virtualizer] choosing measured lower-bound first=%d measuredTotal=%d target=%d", first, measuredTotal, target);
        } else {

            first = std::clamp(target / std::max(1, m_estimatedItemHeight), 0, count - 1);
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Trace,
                "[Virtualizer] choosing estimated jump first=%d estHeight=%d target=%d measuredTotal=%d", first, m_estimatedItemHeight, target, measuredTotal);
        }

    first = std::clamp(first, 0, std::max(0, count - 1));
    const int buffer = 3;

    int startPrefix = m_bit.Sum(first);

    int measuredCountUpToFirst = m_countBit.Sum(first);
    int unmeasuredBefore = first - measuredCountUpToFirst;
    startPrefix += unmeasuredBefore * m_estimatedItemHeight;
    int need = startPrefix + m_viewportHeight;


    int cur = first;
    int prefix = startPrefix;
    auto gen = m_owner->GetGenerator();
    const auto& realItems = m_owner->GetItems();
    const auto& src = m_owner->GetItemsSource();
    while (prefix < need && cur < count) {
        if (m_measuredHeights[cur] < 0) {
            std::shared_ptr<NeoWPF::UI::FrameworkElement> container;
            if (cur < static_cast<int>(realItems.size())) container = realItems[cur];
            else if (gen && cur < static_cast<int>(src.size())) container = gen->Acquire(src[cur], cur, m_owner->GetItemTemplate());
            if (container) {
                container->Measure(m_viewportWidth, 0);
                int h = std::max(1, container->GetDesiredHeight());
                UpdateMeasuredHeight(cur, h);
                if (cur >= static_cast<int>(realItems.size()) && gen) gen->Recycle(cur, container);
            } else {

                break;
            }
        }
        prefix += GetMeasuredHeight(cur);
        ++cur;
    }

    int last = cur - 1;
    m_firstIndex = std::clamp(first - buffer, 0, count - 1);
    m_lastIndex = std::clamp(last + buffer, 0, count - 1);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
        "[Virtualizer] pixel realize first=%d last=%d prefix=%d need=%d measuredTotal=%d unmeasured=%d m_firstIndex=%d m_lastIndex=%d vOffset=%d",
        first, last, startPrefix, need, measuredTotal, std::max(0, count - m_measuredCount), m_firstIndex, m_lastIndex, m_verticalOffset);
}

void ItemsControlVirtualizer::EnsureMeasuredAll() noexcept
{
    if (!m_owner) return;
    const int count = m_owner->GetItemCount();
    if (static_cast<int>(m_measuredHeights.size()) != count) {
        m_measuredHeights.assign(count, -1);
        m_bit.Init(count);
        m_measuredCount = 0;
        m_countBit.Init(count);
        m_sampleSum = 0;
        m_sampleCount = 0;
        m_estimatedItemHeight = m_itemHeight;
    }
    auto gen = m_owner->GetGenerator();
    if (!gen) return;
    const auto& realItems = m_owner->GetItems();
    const auto& src = m_owner->GetItemsSource();
    for (int i = 0; i < count; ++i) {
        if (m_measuredHeights[i] < 0) {
            std::shared_ptr<NeoWPF::UI::FrameworkElement> container;
            if (i < static_cast<int>(realItems.size())) container = realItems[i];
            else if (i < static_cast<int>(src.size())) container = gen->Acquire(src[i], i, m_owner->GetItemTemplate());
            if (container) {
                container->Measure(m_viewportWidth, 0);
                int h = std::max(1, container->GetDesiredHeight());
                UpdateMeasuredHeight(i, h);
                if (i >= static_cast<int>(realItems.size())) gen->Recycle(i, container);
            }
        }
    }
}

void ItemsControlVirtualizer::EnsureMeasuredRange(int first, int last) noexcept
{
    if (!m_owner) return;
    const int count = m_owner->GetItemCount();
    if (first < 0) first = 0;
    if (last >= count) last = count - 1;
    if (first > last) return;
    if (static_cast<int>(m_measuredHeights.size()) != count) {
        m_measuredHeights.assign(count, -1);
        m_bit.Init(count);
        m_measuredCount = 0;
        m_countBit.Init(count);
        m_sampleSum = 0;
        m_sampleCount = 0;
        m_estimatedItemHeight = m_itemHeight;
    }
    auto gen = m_owner->GetGenerator();
    if (!gen) return;
    const auto& realItems = m_owner->GetItems();
    const auto& src = m_owner->GetItemsSource();
    for (int i = first; i <= last; ++i) {
        if (m_measuredHeights[i] < 0) {
            std::shared_ptr<NeoWPF::UI::FrameworkElement> container;
            if (i < static_cast<int>(realItems.size())) container = realItems[i];
            else if (i < static_cast<int>(src.size())) container = gen->Acquire(src[i], i, m_owner->GetItemTemplate());
            if (container) {
                container->Measure(m_viewportWidth, 0);
                int h = std::max(1, container->GetDesiredHeight());
                UpdateMeasuredHeight(i, h);
                if (i >= static_cast<int>(realItems.size())) gen->Recycle(i, container);
            }
        }
    }
}

void ItemsControlVirtualizer::UpdateMeasuredHeight(int index, int height) noexcept
{
    if (index < 0) return;
    const int count = m_owner ? m_owner->GetItemCount() : 0;
    if (index >= count) return;
    if (static_cast<int>(m_measuredHeights.size()) != count) {
        m_measuredHeights.assign(count, -1);
        m_bit.Init(count);
    }
    int prev = m_measuredHeights[index];
    bool wasUnmeasured = (prev < 0);
    if (prev < 0) prev = 0;
    m_measuredHeights[index] = height;
    int delta = height - prev;
    if (delta != 0) {
        m_bit.Update(index + 1, delta);
    }
    if (wasUnmeasured) {
        ++m_measuredCount;
        m_countBit.Update(index + 1, 1);

        if (index < m_sampleSizeLimit && m_sampleCount < m_sampleSizeLimit) {
            m_sampleSum += height;
            ++m_sampleCount;
            m_estimatedItemHeight = static_cast<int>((m_sampleSum + m_sampleCount / 2) / m_sampleCount);
        }
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Trace,
            "[Virtualizer] Measured index=%d height=%d measuredCount=%d estHeight=%d sampleCount=%d", index, height, m_measuredCount, m_estimatedItemHeight, m_sampleCount);
    }
}

int ItemsControlVirtualizer::GetMeasuredHeight(int index) const noexcept
{
    if (index < 0 || static_cast<size_t>(index) >= m_measuredHeights.size()) return m_estimatedItemHeight;
    int v = m_measuredHeights[index];
    return v < 0 ? m_estimatedItemHeight : v;
}

int ItemsControlVirtualizer::GetTotalMeasuredHeight() const noexcept
{
    return m_bit.Total();
}

int ItemsControlVirtualizer::IndexFromOffset(int offset) const noexcept
{
    if (!m_owner) return 0;
    const int count = m_owner->GetItemCount();
    if (count == 0) return 0;
    if (offset <= 0) return 0;

    int low = 0;
    int high = count;
    while (low < high) {
        int mid = (low + high) / 2;
        int midOffset = GetOffsetForIndex(mid);
        if (midOffset <= offset) low = mid + 1;
        else high = mid;
    }
    return low;
}

int ItemsControlVirtualizer::GetOffsetForIndex(int index) const noexcept
{
    if (!m_owner) return 0;
    if (index <= 0) return m_owner->GetPadding();
    int measuredSum = m_bit.Sum(index);
    int measuredCountUpTo = m_countBit.Sum(index);
    int unmeasured = index - measuredCountUpTo;
    return m_owner->GetPadding() + measuredSum + unmeasured * m_estimatedItemHeight;
}

}
