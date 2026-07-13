#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>

namespace NeoWPF::UI {

ItemsControl::ItemsControl()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
}

ItemsControl::~ItemsControl()
{
}

void ItemsControl::SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items)
{
    for (const auto& oldItem : m_items) {
        if (oldItem && oldItem->GetParent() == this) {
            RemoveVisualChild(oldItem);
        }
    }

    m_items = items;
    for (const auto& item : m_items) {
        if (item && item->GetParent() != this) {
            AddVisualChild(item);
        }
    }

    InvalidateVisual();
}

void ItemsControl::AddItem(const std::shared_ptr<FrameworkElement>& item)
{
    if (item) {
        m_items.push_back(item);
        if (item->GetParent() != this) {
            AddVisualChild(item);
        }
        InvalidateVisual();
    }
}

void ItemsControl::ClearItems()
{
    for (const auto& item : m_items) {
        if (item && item->GetParent() == this) {
            RemoveVisualChild(item);
        }
    }

    m_items.clear();
    InvalidateVisual();
}

void ItemsControl::Measure(int availableWidth, int availableHeight)
{
    int totalHeight = m_padding * 2;
    int totalWidth = 0;

    for (const auto& item : m_items) {
        if (!item) {
            continue;
        }

        item->Measure(availableWidth > 0 ? availableWidth - (m_padding * 2) : 0, availableHeight);
        totalWidth = std::max(totalWidth, item->GetDesiredWidth());
        totalHeight += item->GetDesiredHeight() + m_itemSpacing;
    }

    if (!m_items.empty()) {
        totalHeight -= m_itemSpacing;
    }

    const int desiredWidth = totalWidth + m_padding * 2;
    const int desiredHeight = std::max(0, totalHeight);
    SetDesiredSize(ResolveWidth(availableWidth, desiredWidth), ResolveHeight(availableHeight, desiredHeight));
}

void ItemsControl::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    int currentY = m_padding;
    const int contentWidth = std::max(0, finalWidth - m_padding * 2);

    for (const auto& item : m_items) {
        if (!item) {
            continue;
        }

        item->Arrange(contentWidth, std::max(0, finalHeight - m_padding * 2));
        item->SetLayoutOffset(m_padding, currentY);
        currentY += item->GetDesiredHeight() + m_itemSpacing;
    }
}

void ItemsControl::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    for (const auto& item : m_items) {
        if (item) {
            item->BuildRenderCommands(commands);
        }
    }
}

}
