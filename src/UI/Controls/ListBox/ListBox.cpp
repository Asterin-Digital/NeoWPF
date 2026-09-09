#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

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

ListBox::ListBox() = default;

ListBox::~ListBox() = default;

void ListBox::AddItem(const std::shared_ptr<FrameworkElement>& item)
{
    AddSelectionHandlerToItem(this, item);
    ItemsControl::AddItem(item);
}

void ListBox::SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items)
{
    for (const auto& item : items) {
        AddSelectionHandlerToItem(this, item);
    }
    ItemsControl::SetItems(items);
}

std::shared_ptr<FrameworkElement> ListBox::GetContainerForItem(const std::any& data, int index)
{
    (void)index;
    auto container = std::make_shared<ListBoxItem>();
    container->SetDataContext(data);
    AddSelectionHandlerToItem(this, container);
    return container;
}

void ListBox::PrepareContainer(const std::shared_ptr<FrameworkElement>& container,
                               const std::any& data,
                               int index)
{
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
    (void)oldIndex;
    (void)newIndex;
    (void)oldItem;
    (void)newItem;

    const auto items = GetItems();
    for (const auto& itemVisual : items) {
        if (!itemVisual) {
            continue;
        }

        auto listBoxItem = std::dynamic_pointer_cast<ListBoxItem>(itemVisual);
        if (!listBoxItem) {
            continue;
        }

        const auto data = listBoxItem->GetDataContext();
        listBoxItem->SetIsSelected(IsSelected(data));
    }
}

}
