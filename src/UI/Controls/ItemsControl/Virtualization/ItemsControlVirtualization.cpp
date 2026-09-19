#include <NeoWPF/UI/Controls/ItemsControl/Virtualization/ItemsControlVirtualization.h>
#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <vector>
#include <memory>

namespace NeoWPF::UI::Controls::ItemsControl::Virtualization {

ItemsControlVirtualizer::ItemsControlVirtualizer(NeoWPF::UI::ItemsControl* owner, double itemHeight) noexcept
    : m_owner(owner)
    , m_itemHeight((std::max)(1.0, itemHeight))
{
    m_estimatedItemHeight = m_itemHeight;
}

double ItemsControlVirtualizer::GetExtentWidth() const noexcept
{
    return m_owner ? m_owner->GetWidth() : 0;
}

double ItemsControlVirtualizer::GetExtentHeight() const noexcept
{
    if (!m_owner) return 0;
    const int count = m_owner->GetItemCount();
    double measuredTotal = m_bit.Total();
    int unmeasured = (std::max)(0, count - m_measuredCount);
    double total = measuredTotal + unmeasured * m_estimatedItemHeight;
    const double paddingTotal = m_owner->GetPadding().Horizontal() + m_owner->GetPadding().Vertical();
    double result = total + (std::max)(0.0, paddingTotal);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
        "[Virtualizer.GetExtentHeight] count=%d measured=%d estHeight=%g measuredTotal=%g unmeasured=%d total=%g",
        count, m_measuredCount, m_estimatedItemHeight, measuredTotal, unmeasured, result);
    return result;
}

double ItemsControlVirtualizer::GetViewportWidth() const noexcept { return m_viewportWidth; }
double ItemsControlVirtualizer::GetViewportHeight() const noexcept { return m_viewportHeight; }

double ItemsControlVirtualizer::GetHorizontalOffset() const noexcept { return m_horizontalOffset; }
double ItemsControlVirtualizer::GetVerticalOffset() const noexcept {
    if (m_logicalScrolling) return GetOffsetForIndex(m_verticalIndexOffset);
    return m_verticalOffset;
}

void ItemsControlVirtualizer::SetHorizontalOffset(double offset) noexcept
{
    m_horizontalOffset = (std::max)(0.0, offset);
}

void ItemsControlVirtualizer::SetVerticalOffset(double offset) noexcept
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
        "[Virtualizer] SetVerticalOffset requested=%d logical=%d", offset, m_logicalScrolling);

    if (m_logicalScrolling) {
        const int count = m_owner ? m_owner->GetItemCount() : 0;
        const int clampedIndex = std::clamp(static_cast<int>(offset), 0, (std::max)(0, count - 1));
        if (clampedIndex == m_verticalIndexOffset) return;
        m_verticalIndexOffset = clampedIndex;
        RecomputeRealizedRange();
        if (m_owner) m_owner->InvalidateVisual();
        return;
    }

    const double extent = GetExtentHeight();
    const double maxOffset = (std::max)(0.0, extent - m_viewportHeight);
    const double clamped = std::clamp(offset, 0.0, maxOffset);
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

void ItemsControlVirtualizer::SetViewportSize(double width, double height) noexcept
{
    m_viewportWidth = (std::max)(0.0, width);
    m_viewportHeight = (std::max)(0.0, height);
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
        double usedHeight = 0.0;
        while (usedHeight < m_viewportHeight && last < count) {
            if (m_measuredHeights[last] < 0) {
                MeasureItem(last);
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

    const auto padding = m_owner->GetPadding();
    const double target = (std::max)(0.0, m_verticalOffset - padding.Top);

    double measuredTotal = m_bit.Total();
    int first = 0;
        if (measuredTotal > target) {

            first = m_bit.LowerBound(target);
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Trace,
                "[Virtualizer] choosing measured lower-bound first=%d measuredTotal=%d target=%d", first, measuredTotal, target);
        } else {

            first = std::clamp(static_cast<int>(target / (std::max)(1.0, m_estimatedItemHeight)), 0, count - 1);
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Trace,
                "[Virtualizer] choosing estimated jump first=%d estHeight=%d target=%d measuredTotal=%d", first, m_estimatedItemHeight, target, measuredTotal);
        }

    first = std::clamp(first, 0, std::max(0, count - 1));
    const int buffer = 3;

    double startPrefix = m_bit.Sum(first);

    int measuredCountUpToFirst = static_cast<int>(m_countBit.Sum(first));
    int unmeasuredBefore = first - measuredCountUpToFirst;
    startPrefix += unmeasuredBefore * m_estimatedItemHeight;
    double need = startPrefix + m_viewportHeight;


    int cur = first;
    double prefix = startPrefix;
    while (prefix < need && cur < count) {
        if (m_measuredHeights[cur] < 0) {
            if (MeasureItem(cur) <= 0) {
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
    for (int i = 0; i < count; ++i) {
        if (m_measuredHeights[i] < 0) {
            MeasureItem(i);
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
    for (int i = first; i <= last; ++i) {
        if (m_measuredHeights[i] < 0) {
            MeasureItem(i);
        }
    }
}

double ItemsControlVirtualizer::MeasureItem(int index) noexcept
{
    if (!m_owner || index < 0 || index >= m_owner->GetItemCount()) {
        return 0;
    }

    const auto& realItems = m_owner->GetItems();
    std::shared_ptr<NeoWPF::UI::FrameworkElement> container;
    bool temporaryContainer = false;

    if (index < static_cast<int>(realItems.size())) {
        container = realItems[index];
    } else {
        const auto& source = m_owner->GetItemsSource();
        auto generator = m_owner->GetGenerator();
        if (generator && index < static_cast<int>(source.size())) {
            container = generator->Acquire(source[index], index, m_owner->GetItemTemplate());
            temporaryContainer = true;
        }
    }

    if (!container) {
        return 0;
    }

    container->Measure(m_viewportWidth, 0);
    const double height = (std::max)(1.0, container->GetDesiredHeight());
    UpdateMeasuredHeight(index, height);

    if (temporaryContainer) {
        if (auto generator = m_owner->GetGenerator()) {
            generator->Recycle(index, container);
        }
    }

    return height;
}

void ItemsControlVirtualizer::UpdateMeasuredHeight(int index, double height) noexcept
{
    if (index < 0) return;
    const int count = m_owner ? m_owner->GetItemCount() : 0;
    if (index >= count) return;
    if (static_cast<int>(m_measuredHeights.size()) != count) {
        m_measuredHeights.assign(count, -1);
        m_bit.Init(count);
    }
    double prev = m_measuredHeights[index];
    bool wasUnmeasured = (prev < 0);
    if (prev < 0) prev = 0;
    m_measuredHeights[index] = height;
    double delta = height - prev;
    if (delta != 0) {
        m_bit.Update(index + 1, delta);
    }
    if (wasUnmeasured) {
        ++m_measuredCount;
        m_countBit.Update(index + 1, 1);

        if (index < m_sampleSizeLimit && m_sampleCount < m_sampleSizeLimit) {
            m_sampleSum += height;
            ++m_sampleCount;
            m_estimatedItemHeight = m_sampleSum / static_cast<double>(m_sampleCount);
        }
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Trace,
            "[Virtualizer] Measured index=%d height=%d measuredCount=%d estHeight=%d sampleCount=%d", index, height, m_measuredCount, m_estimatedItemHeight, m_sampleCount);
    }
}

double ItemsControlVirtualizer::GetMeasuredHeight(int index) const noexcept
{
    if (index < 0 || static_cast<size_t>(index) >= m_measuredHeights.size()) return m_estimatedItemHeight;
    double v = m_measuredHeights[index];
    return v < 0 ? m_estimatedItemHeight : v;
}

double ItemsControlVirtualizer::GetTotalMeasuredHeight() const noexcept
{
    return m_bit.Total();
}

int ItemsControlVirtualizer::IndexFromOffset(double offset) const noexcept
{
    if (!m_owner) return 0;
    const int count = m_owner->GetItemCount();
    if (count == 0) return 0;
    if (offset <= 0) return 0;

    int low = 0;
    int high = count;
    while (low < high) {
        int mid = (low + high) / 2;
        double midOffset = GetOffsetForIndex(mid);
        if (midOffset <= offset) low = mid + 1;
        else high = mid;
    }
    return low;
}

double ItemsControlVirtualizer::GetOffsetForIndex(int index) const noexcept
{
    if (!m_owner) {
        return 0;
    }

    if (index <= 0) {
        return m_owner->GetPadding().Top;
    }

    double measuredSum = m_bit.Sum(index);
    int measuredCountUpTo = static_cast<int>(m_countBit.Sum(index));
    int unmeasured = index - measuredCountUpTo;
    return m_owner->GetPadding().Top + measuredSum + unmeasured * m_estimatedItemHeight;
}

}
