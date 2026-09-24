/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Layout/Scrolling/IScrollInfo.h>
#include <memory>
#include <vector>
#include <algorithm>

namespace NeoWPF::UI { class ItemsControl; }

namespace NeoWPF::UI::Controls::ItemsControl::Virtualization {

class ItemsControlVirtualizer : public Layout::Scrolling::IScrollInfo, public std::enable_shared_from_this<ItemsControlVirtualizer> {
public:
    explicit ItemsControlVirtualizer(NeoWPF::UI::ItemsControl* owner, double itemHeight = 24.0) noexcept;

    void SetItemHeight(double height) noexcept { m_itemHeight = (std::max)(1.0, height); }


    double GetExtentWidth() const noexcept override;
    double GetExtentHeight() const noexcept override;

    double GetViewportWidth() const noexcept override;
    double GetViewportHeight() const noexcept override;

    double GetHorizontalOffset() const noexcept override;
    double GetVerticalOffset() const noexcept override;

    void SetHorizontalOffset(double offset) noexcept override;
    void SetVerticalOffset(double offset) noexcept override;

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

    void SetViewportSize(double width, double height) noexcept;
    double GetItemHeight() const noexcept { return m_itemHeight; }
    
    void SetLogicalScrolling(bool logical) noexcept { m_logicalScrolling = logical; }
    bool GetLogicalScrolling() const noexcept { return m_logicalScrolling; }


    void RecomputeRealizedRange() noexcept;
    double GetOffsetForIndex(int index) const noexcept;
    double GetMeasuredHeight(int index) const noexcept;
    void UpdateMeasuredHeight(int index, double height) noexcept;

private:
    NeoWPF::UI::ItemsControl* m_owner;
    double m_itemHeight;
    double m_viewportWidth = 0.0;
    double m_viewportHeight = 0.0;
    double m_horizontalOffset = 0.0;
    double m_verticalOffset = 0.0;

    int m_firstIndex = 0;
    int m_lastIndex = -1;
    int m_verticalIndexOffset = 0;
    bool m_logicalScrolling = false;


    std::vector<double> m_measuredHeights;
    class Fenwick {
    public:
        void Init(int n) { m_n = n; m_bit.assign(n + 1, 0.0); }
        void Update(int idx, double delta) {
            for (int i = idx; i <= m_n; i += i & -i) m_bit[i] += delta;
        }
        double Sum(int idx) const {
            double s = 0.0;
            for (int i = idx; i > 0; i -= i & -i) s += m_bit[i];
            return s;
        }

        int LowerBound(double value) const {
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
        double Total() const { return Sum(m_n); }
    private:
        int m_n = 0;
        std::vector<double> m_bit;
    };
    Fenwick m_bit;
    Fenwick m_countBit;
    int m_measuredCount = 0;
    double m_sampleSum = 0.0;
    int m_sampleCount = 0;
    int m_sampleSizeLimit = 5;
    double m_estimatedItemHeight = 0.0;


    void EnsureMeasuredAll() noexcept;
    void EnsureMeasuredRange(int first, int last) noexcept;
    double MeasureItem(int index) noexcept;
    double GetTotalMeasuredHeight() const noexcept;
    int IndexFromOffset(double offset) const noexcept;
};

}
