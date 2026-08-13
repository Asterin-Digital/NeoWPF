#pragma once

#include <NeoWPF/UI/Layout/Scrolling/IScrollInfo.h>
#include <memory>
#include <vector>
#include <algorithm>

namespace NeoWPF::UI { class ItemsControl; }

namespace NeoWPF::UI::Controls::ItemsControl::Virtualization {

class ItemsControlVirtualizer : public Layout::Scrolling::IScrollInfo, public std::enable_shared_from_this<ItemsControlVirtualizer> {
public:
    explicit ItemsControlVirtualizer(NeoWPF::UI::ItemsControl* owner, int itemHeight = 24) noexcept;

    void SetItemHeight(int height) noexcept { m_itemHeight = std::max(1, height); }


    int GetExtentWidth() const noexcept override;
    int GetExtentHeight() const noexcept override;

    int GetViewportWidth() const noexcept override;
    int GetViewportHeight() const noexcept override;

    int GetHorizontalOffset() const noexcept override;
    int GetVerticalOffset() const noexcept override;

    void SetHorizontalOffset(int offset) noexcept override;
    void SetVerticalOffset(int offset) noexcept override;

    void LineLeft() noexcept override;
    void LineRight() noexcept override;
    void LineUp() noexcept override;
    void LineDown() noexcept override;

    void PageLeft() noexcept override;
    void PageRight() noexcept override;
    void PageUp() noexcept override;
    void PageDown() noexcept override;

    void MouseWheel(int delta) noexcept override;

    int GetFirstRealizedIndex() const noexcept { return m_firstIndex; }
    int GetLastRealizedIndex() const noexcept { return m_lastIndex; }

    void SetViewportSize(int width, int height) noexcept;
    int GetItemHeight() const noexcept { return m_itemHeight; }
    
    void SetLogicalScrolling(bool logical) noexcept { m_logicalScrolling = logical; }
    bool GetLogicalScrolling() const noexcept { return m_logicalScrolling; }


    void RecomputeRealizedRange() noexcept;
    int GetOffsetForIndex(int index) const noexcept;
    int GetMeasuredHeight(int index) const noexcept;
    void UpdateMeasuredHeight(int index, int height) noexcept;

private:
    NeoWPF::UI::ItemsControl* m_owner;
    int m_itemHeight;
    int m_viewportWidth = 0;
    int m_viewportHeight = 0;
    int m_horizontalOffset = 0;
    int m_verticalOffset = 0;

    int m_firstIndex = 0;
    int m_lastIndex = -1;
    int m_verticalIndexOffset = 0;
    bool m_logicalScrolling = false;


    std::vector<int> m_measuredHeights;
    class Fenwick {
    public:
        void Init(int n) { m_n = n; m_bit.assign(n + 1, 0); }
        void Update(int idx, int delta) {
            for (int i = idx; i <= m_n; i += i & -i) m_bit[i] += delta;
        }
        int Sum(int idx) const {
            int s = 0;
            for (int i = idx; i > 0; i -= i & -i) s += m_bit[i];
            return s;
        }

        int LowerBound(int value) const {
            int idx = 0;
            int bitMask = 1;
            while (bitMask <= m_n) bitMask <<= 1;
            for (int k = bitMask; k > 0; k >>= 1) {
                int next = idx + k;
                if (next <= m_n && m_bit[next] <= value) {
                    idx = next;
                    value -= m_bit[next];
                }
            }
            return idx;
        }
        int Total() const { return Sum(m_n); }
    private:
        int m_n = 0;
        std::vector<int> m_bit;
    };
    Fenwick m_bit;
    Fenwick m_countBit;
    int m_measuredCount = 0;
    long long m_sampleSum = 0;
    int m_sampleCount = 0;
    int m_sampleSizeLimit = 5;
    int m_estimatedItemHeight = 0; // haha i hate virtualization


    void EnsureMeasuredAll() noexcept;
    void EnsureMeasuredRange(int first, int last) noexcept;
    int GetTotalMeasuredHeight() const noexcept;
    int IndexFromOffset(int offset) const noexcept;
};

}
