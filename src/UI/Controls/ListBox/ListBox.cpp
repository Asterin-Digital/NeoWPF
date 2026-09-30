#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::UI {

namespace {
void AddSelectionHandlerToItem(ListBox* owner, const std::shared_ptr<FrameworkElement>& element)
{
    if (!owner || !element) {
        return;
    }

    auto listBoxItem = std::dynamic_pointer_cast<ListBoxItem>(element);
    if (!listBoxItem) {
        return;
    }

    listBoxItem->SetOwner(owner);
    if (listBoxItem->HasRoutedEventHandler(NeoWPF::Input::Core::MouseUpEvent())) {
        return;
    }

    listBoxItem->AddHandler(NeoWPF::Input::Core::MouseUpEvent(), [owner](UIElement* sender, NeoWPF::Input::Core::RoutedEventArgs* args) {
        if (!sender || !args) {
            return;
        }

        auto* item = dynamic_cast<ListBoxItem*>(sender);
        if (!item) {
            return;
        }

        const auto data = item->GetDataContext();
        if (data.has_value()) {
            owner->SetSelectedItem(data);
            args->SetHandled(true);
        }
    });
}
}

ListBox::ListBox()
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace, "[ListBox] Constructed listBox=%p", this);
}

ListBox::~ListBox()
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace, "[ListBox] Destroyed listBox=%p", this);
}

void ListBox::AddItem(std::shared_ptr<FrameworkElement> item)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBox] AddItem listBox=%p item=%p beforeCount=%zu parent=%p",
        this, item.get(), GetItems().size(), item ? item->GetParent() : nullptr);
    AddSelectionHandlerToItem(this, item);
    ItemsControl::AddItem(item);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBox] AddItem complete listBox=%p item=%p afterCount=%zu parent=%p",
        this, item.get(), GetItems().size(), item ? item->GetParent() : nullptr);
}

void ListBox::SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBox] SetItems listBox=%p count=%zu", this, items.size());
    for (const auto& item : items) {
        AddSelectionHandlerToItem(this, item);
    }
    ItemsControl::SetItems(items);
}

void ListBox::SyncSelection(const std::any& item)
{
    SetSelectedItem(item);
}

std::shared_ptr<FrameworkElement> ListBox::GetContainerForItem(const std::any& data, int index)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBox] GetContainerForItem listBox=%p index=%d dataType=%s",
        this, index, data.has_value() ? data.type().name() : "null");
    auto container = std::make_shared<ListBoxItem>();
    container->SetDataContext(data);
    AddSelectionHandlerToItem(this, container);
    return container;
}

void ListBox::PrepareContainer(const std::shared_ptr<FrameworkElement>& container,
                               const std::any& data,
                               int index)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBox] PrepareContainer listBox=%p index=%d container=%p dataType=%s",
        this, index, container.get(), data.has_value() ? data.type().name() : "null");
    if (!container) {
        return;
    }

    container->SetDataContext(data);

    if (auto listBoxItem = std::dynamic_pointer_cast<ListBoxItem>(container)) {
        listBoxItem->SetOwner(this);
        listBoxItem->SetIsSelected(IsSelected(data));
        AddSelectionHandlerToItem(this, container);
        (void)index;
    }
}

void ListBox::OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBox] OnSelectionChanged listBox=%p oldIndex=%d newIndex=%d itemCount=%d",
        this, oldIndex, newIndex, GetItemCount());
    (void)oldIndex;
    (void)newIndex;
    (void)oldItem;
    (void)newItem;

    const auto items = GetVisualChildren();
    for (auto* itemVisual : items) {
        if (!itemVisual) {
            continue;
        }

        auto* listBoxItem = dynamic_cast<ListBoxItem*>(itemVisual);
        if (!listBoxItem) {
            continue;
        }

        const auto data = listBoxItem->GetDataContext();
        listBoxItem->SetIsSelected(IsSelected(data));
    }
}

}
