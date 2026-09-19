#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <NeoWPF/UI/Controls/ItemsControl/Virtualization/ItemTemplate.h>
#include <any>
#include <unordered_map>

namespace NeoWPF::UI {

ItemsControl::ItemsControl()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetClipToBounds(true);
    m_generator = std::make_shared<Controls::ItemsControl::Virtualization::ItemContainerGenerator>(this);

}

void ItemsControl::SetItemsSource(const std::vector<std::any>& source)
{
    for (const auto& oldItem : m_items) {
        if (oldItem && oldItem->GetParent() == this) {
            RemoveVisualChild(oldItem);
        }
    }
    m_items.clear();
    ClearRealizedItems();

    m_itemsSource = source;
    m_hasItemsSource = true;
    if (m_virtualization) {
        m_virtualization->SetHorizontalOffset(0);
        m_virtualization->SetVerticalOffset(0);
        m_virtualization->RecomputeRealizedRange();
    } else {
        RealizeSourceItems();
    }
    InvalidateMeasure();
    InvalidateArrange();
    InvalidateVisual();
}

void ItemsControl::SetItemTemplate(const Controls::ItemsControl::Virtualization::ItemTemplate& templ)
{
    m_itemTemplate = templ;
    ClearRealizedItems();
    if (m_hasItemsSource && !m_virtualization) {
        RealizeSourceItems();
    } else if (m_virtualization) {
        m_virtualization->RecomputeRealizedRange();
    }
    InvalidateMeasure();
    InvalidateArrange();
    InvalidateVisual();
}

std::shared_ptr<FrameworkElement> ItemsControl::GetContainerForItem(const std::any& data, int index)
{
    (void)data;
    (void)index;
    return nullptr;
}

void ItemsControl::PrepareContainer(const std::shared_ptr<FrameworkElement>& container,
                                    const std::any& data,
                                    int index)
{
    (void)container;
    (void)data;
    (void)index;
}

void ItemsControl::ApplyItemTemplate(const std::shared_ptr<FrameworkElement>& container,
                                     const std::any& data,
                                     int index,
                                     const Controls::ItemsControl::Virtualization::ItemTemplate& templ)
{
    (void)container;
    (void)data;
    (void)index;
    (void)templ;
}

ItemsControl::~ItemsControl()
{
}

void ItemsControl::SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items)
{
    ClearRealizedItems();
    for (const auto& oldItem : m_items) {
        if (oldItem && oldItem->GetParent() == this) {
            RemoveVisualChild(oldItem);
        }
    }

    m_itemsSource.clear();
    m_hasItemsSource = false;
    m_items = items;
    for (const auto& item : m_items) {
        if (item && item->GetParent() != this) {
            AddVisualChild(item);
        }
    }

    InvalidateMeasure();
    InvalidateArrange();
    InvalidateVisual();
}

void ItemsControl::AddItem(std::shared_ptr<FrameworkElement> item)
{
    if (item && !m_hasItemsSource) {
        m_items.push_back(item);
        if (item->GetParent() != this) {
            AddVisualChild(item);
        }
        InvalidateMeasure();
        InvalidateArrange();
        InvalidateVisual();
    }
}

void ItemsControl::ClearItems()
{
    ClearRealizedItems();
    for (const auto& item : m_items) {
        if (item && item->GetParent() == this) {
            RemoveVisualChild(item);
        }
    }

    m_items.clear();
    m_itemsSource.clear();
    m_hasItemsSource = false;
    InvalidateMeasure();
    InvalidateArrange();
    InvalidateVisual();
}

void ItemsControl::ClearRealizedItems() noexcept
{
    for (auto& entry : m_realizedItems) {
        if (!entry.second) {
            continue;
        }
        if (entry.second->GetParent() == this) {
            RemoveVisualChild(entry.second);
        }
        if (m_generator) {
            m_generator->Recycle(entry.first, entry.second);
        }
    }
    m_realizedItems.clear();
}

void ItemsControl::RealizeSourceItems()
{
    if (!m_hasItemsSource || !m_itemTemplate) {
        return;
    }

    const int count = static_cast<int>(m_itemsSource.size());
    for (int i = 0; i < count; ++i) {
        auto element = m_generator
            ? m_generator->Acquire(m_itemsSource[i], i, m_itemTemplate)
            : std::shared_ptr<FrameworkElement>{};
        if (element) {
            m_realizedItems[i] = element;
            if (element->GetParent() != this) {
                AddVisualChild(element);
            }
        }
    }
}

void ItemsControl::Measure(double availableWidth, double availableHeight)
{
    const double itemSpacing = GetVerticalSpacing();

    const double contentWidth = availableWidth > 0.0 ? (std::max)(0.0, availableWidth - (m_padding * 2)) : 0.0;
    if (m_virtualization) {
        m_virtualization->SetViewportSize(contentWidth, availableHeight);


        double totalWidth = 0.0;
        int first = m_virtualization->GetFirstRealizedIndex();
        int last = m_virtualization->GetLastRealizedIndex();

        if (m_hasItemsSource) {
            RealizeRange(first, last);
            for (int i = first; i <= last; ++i) {
                auto it = m_realizedItems.find(i);
                if (it == m_realizedItems.end() || !it->second) continue;
                it->second->Measure(contentWidth, 0);
                totalWidth = (std::max)(totalWidth, it->second->GetDesiredWidth());
            }
        } else {
            const int count = static_cast<int>(m_items.size());
            if (first <= last && count > 0) {
                for (int i = first; i <= last; ++i) {
                    const auto& item = m_items[i];
                    if (!item) continue;
                    item->Measure(contentWidth, 0);
                    totalWidth = (std::max)(totalWidth, item->GetDesiredWidth());
                }
            }
        }

        const double desiredWidth = (std::max)(totalWidth, contentWidth) + m_padding * 2;
        const double desiredHeight = (std::max)(0.0, m_virtualization->GetExtentHeight());
        SetDesiredSize(ResolveWidth(availableWidth, desiredWidth), ResolveHeight(availableHeight, desiredHeight));
        return;
    }

    double totalHeight = m_padding * 2;
    double totalWidth = 0.0;

    if (m_hasItemsSource) {
        const int count = static_cast<int>(m_itemsSource.size());
        for (int i = 0; i < count; ++i) {
            auto it = m_realizedItems.find(i);
            if (it == m_realizedItems.end() || !it->second) {
                continue;
            }

            it->second->Measure(contentWidth, 0);
            totalWidth = (std::max)(totalWidth, it->second->GetDesiredWidth());
            totalHeight += it->second->GetDesiredHeight() + itemSpacing;
        }

        if (count > 0) {
            totalHeight -= itemSpacing;
        }
    } else {
        for (const auto& item : m_items) {
            if (!item) {
                continue;
            }

            item->Measure(contentWidth, 0);
            totalWidth = (std::max)(totalWidth, item->GetDesiredWidth());
            totalHeight += item->GetDesiredHeight() + itemSpacing;
        }

        if (!m_items.empty()) {
            totalHeight -= itemSpacing;
        }
    }

    const double desiredWidth = (std::max)(totalWidth, contentWidth) + m_padding * 2;
    const double desiredHeight = (std::max)(0.0, totalHeight);
    SetDesiredSize(ResolveWidth(availableWidth, desiredWidth), ResolveHeight(availableHeight, desiredHeight));
}

void ItemsControl::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    const double itemSpacing = GetVerticalSpacing();
    const double contentWidth = (std::max)(0.0, finalWidth - m_padding * 2);

    if (m_virtualization) {
        const int first = m_virtualization->GetFirstRealizedIndex();
        const int last = m_virtualization->GetLastRealizedIndex();
        const double itemHeight = m_virtualization->GetItemHeight();
        const double vOffset = m_virtualization->GetVerticalOffset();


        if (m_hasItemsSource) {
            RealizeRange(first, last);
            for (int i = first; i <= last; ++i) {
                auto it = m_realizedItems.find(i);
                if (it == m_realizedItems.end() || !it->second) continue;
                const double h = it->second->GetDesiredHeight();
                const double y = m_virtualization->GetOffsetForIndex(i) - vOffset;
                m_virtualization->UpdateMeasuredHeight(i, (std::max)(1.0, h));
                it->second->Arrange(contentWidth, h);
                it->second->SetLayoutOffset(m_padding, y);
                auto dataCtx = it->second->GetDataContext();
                std::string dataStr = dataCtx.has_value() ? dataCtx.type().name() : "null";
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
                    "[ItemsControl.Arrange] index=%d y=%d h=%d dataCtx=%s", i, y, h, dataStr.c_str());
            }
        } else {
            for (int i = first; i <= last; ++i) {
                if (i < 0 || i >= static_cast<int>(m_items.size())) continue;
                const auto& item = m_items[i];
                if (!item) continue;
                const double y = m_virtualization->GetOffsetForIndex(i) - vOffset;
                const double h = m_virtualization->GetMeasuredHeight(i);
                item->Arrange(contentWidth, h);
                item->SetLayoutOffset(m_padding, y);
            }
        }

        return;
    }

    double currentY = m_padding;

    if (m_hasItemsSource) {
        const int count = static_cast<int>(m_itemsSource.size());
        for (int i = 0; i < count; ++i) {
            auto it = m_realizedItems.find(i);
            if (it == m_realizedItems.end() || !it->second) {
                continue;
            }

            const double itemHeight = (std::max)(0.0, it->second->GetDesiredHeight());
            it->second->Arrange(contentWidth, itemHeight);
            it->second->SetLayoutOffset(m_padding, currentY);
            currentY += itemHeight + itemSpacing;
        }
    } else {
        for (const auto& item : m_items) {
            if (!item) {
                continue;
            }

            const double itemHeight = (std::max)(0.0, item->GetDesiredHeight());
            item->Arrange(contentWidth, itemHeight);
            item->SetLayoutOffset(m_padding, currentY);
            currentY += itemHeight + itemSpacing;
        }
    }
}

void ItemsControl::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_virtualization) {
        const int first = m_virtualization->GetFirstRealizedIndex();
        const int last = m_virtualization->GetLastRealizedIndex();

        if (m_hasItemsSource) {
            for (int i = first; i <= last; ++i) {
                auto it = m_realizedItems.find(i);
                if (it != m_realizedItems.end() && it->second) it->second->BuildRenderCommands(commands);
            }
        } else {
            for (int i = first; i <= last; ++i) {
                if (i < 0 || i >= static_cast<int>(m_items.size())) continue;
                const auto& item = m_items[i];
                if (item) item->BuildRenderCommands(commands);
            }
        }
        return;
    }

    if (m_hasItemsSource) {
        const int count = static_cast<int>(m_itemsSource.size());
        for (int i = 0; i < count; ++i) {
            auto it = m_realizedItems.find(i);
            if (it != m_realizedItems.end() && it->second) {
                it->second->BuildRenderCommands(commands);
            }
        }
    } else {
        for (const auto& item : m_items) {
            if (item) {
                item->BuildRenderCommands(commands);
            }
        }
    }
}

void ItemsControl::RealizeRange(int first, int last)
{
    if (!m_hasItemsSource || !m_itemTemplate) return;
    const int count = static_cast<int>(m_itemsSource.size());
    first = std::clamp(first, 0, std::max(0, count - 1));
    last = std::clamp(last, 0, std::max(0, count - 1));


    for (int i = first; i <= last; ++i) {
        if (m_realizedItems.find(i) == m_realizedItems.end()) {
            std::shared_ptr<FrameworkElement> element;
            if (m_generator) {
                element = m_generator->Acquire(m_itemsSource[i], i, m_itemTemplate);
            } else if (m_itemTemplate) {
                element = m_itemTemplate(m_itemsSource[i], i);
            }

            if (element) {
                m_realizedItems[i] = element;
                if (element->GetParent() != this) AddVisualChild(element);
            }
        }
    }

    std::vector<int> toRemove;
    for (auto& kv : m_realizedItems) {
        int idx = kv.first;
        if (idx < first || idx > last) toRemove.push_back(idx);
    }
    for (int idx : toRemove) {
        auto it = m_realizedItems.find(idx);
        if (it != m_realizedItems.end()) {
            if (it->second) {
                if (it->second->GetParent() == this) RemoveVisualChild(it->second);
                if (m_generator) m_generator->Recycle(idx, it->second);
            }
            m_realizedItems.erase(it);
        }
    }
}


double ItemsControl::GetExtentWidth() const noexcept
{
    if (m_virtualization) return m_virtualization->GetExtentWidth();
    return GetDesiredWidth();
}

double ItemsControl::GetExtentHeight() const noexcept
{
    if (m_virtualization) {
        double result = m_virtualization->GetExtentHeight();
        int count = GetItemCount();
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
            "[ItemsControl] GetExtentHeight count=%d virt=%d", count, result);
        return result;
    }
    double result = GetDesiredHeight();
    int count = GetItemCount();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
        "[ItemsControl] GetExtentHeight count=%d (no virt)=%d", count, result);
    return result;
}

double ItemsControl::GetViewportWidth() const noexcept
{
    if (m_virtualization) return m_virtualization->GetViewportWidth();
    return GetFinalWidth();
}

double ItemsControl::GetViewportHeight() const noexcept
{
    if (m_virtualization) return m_virtualization->GetViewportHeight();
    return GetFinalHeight();
}

double ItemsControl::GetHorizontalOffset() const noexcept
{
    if (m_virtualization) return m_virtualization->GetHorizontalOffset();
    return 0;
}

double ItemsControl::GetVerticalOffset() const noexcept
{
    if (m_virtualization) return m_virtualization->GetVerticalOffset();
    return 0;
}

void ItemsControl::SetHorizontalOffset(double offset) noexcept
{
    if (m_virtualization) m_virtualization->SetHorizontalOffset(offset);
}

void ItemsControl::SetVerticalOffset(double offset) noexcept
{
    if (m_virtualization) m_virtualization->SetVerticalOffset(offset);
}

void ItemsControl::LineLeft() noexcept { if (m_virtualization) m_virtualization->LineLeft(); }
void ItemsControl::LineRight() noexcept { if (m_virtualization) m_virtualization->LineRight(); }
void ItemsControl::LineUp() noexcept { if (m_virtualization) m_virtualization->LineUp(); }
void ItemsControl::LineDown() noexcept { if (m_virtualization) m_virtualization->LineDown(); }

void ItemsControl::PageLeft() noexcept { if (m_virtualization) m_virtualization->PageLeft(); }
void ItemsControl::PageRight() noexcept { if (m_virtualization) m_virtualization->PageRight(); }
void ItemsControl::PageUp() noexcept { if (m_virtualization) m_virtualization->PageUp(); }
void ItemsControl::PageDown() noexcept { if (m_virtualization) m_virtualization->PageDown(); }

void ItemsControl::MouseWheel(int delta) noexcept { if (m_virtualization) m_virtualization->MouseWheel(delta); }

void ItemsControl::SetVirtualization(bool enabled) noexcept
{
    if (enabled) {
        if (!m_virtualization) {
            m_virtualization = std::make_shared<Controls::ItemsControl::Virtualization::ItemsControlVirtualizer>(this);
        }
        m_virtualization->RecomputeRealizedRange();
    } else {
        ClearRealizedItems();
        m_virtualization = nullptr;
        if (m_hasItemsSource) {
            RealizeSourceItems();
        }
    }

    InvalidateMeasure();
    InvalidateArrange();
    InvalidateVisual();
}

}
