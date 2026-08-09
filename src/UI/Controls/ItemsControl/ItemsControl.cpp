#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>

namespace NeoWPF::UI {

ItemsControl::ItemsControl()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetClipToBounds(true);
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
    const int itemSpacing = GetVerticalSpacing();
    int totalHeight = m_padding * 2;
    int totalWidth = 0;
    const int contentWidth = availableWidth > 0 ? std::max(0, availableWidth - (m_padding * 2)) : 0;

    for (const auto& item : m_items) {
        if (!item) {
            continue;
        }

        item->Measure(contentWidth, 0);
        totalWidth = std::max(totalWidth, item->GetDesiredWidth());
        totalHeight += item->GetDesiredHeight() + itemSpacing;
    }

    if (!m_items.empty()) {
        totalHeight -= itemSpacing;
    }

    const int desiredWidth = std::max(totalWidth, contentWidth) + m_padding * 2;
    const int desiredHeight = std::max(0, totalHeight);
    SetDesiredSize(ResolveWidth(availableWidth, desiredWidth), ResolveHeight(availableHeight, desiredHeight));
}

void ItemsControl::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    const int itemSpacing = GetVerticalSpacing();
    int currentY = m_padding;
    const int contentWidth = std::max(0, finalWidth - m_padding * 2);

    for (const auto& item : m_items) {
        if (!item) {
            continue;
        }

        const int itemHeight = std::max(0, item->GetDesiredHeight());
        item->Arrange(contentWidth, itemHeight);
        item->SetLayoutOffset(m_padding, currentY);
        currentY += itemHeight + itemSpacing;
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
