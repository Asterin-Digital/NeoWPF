#include <NeoWPF/UI/Controls/Selector/Selector.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <algorithm>

namespace NeoWPF::UI {

Selector::Selector() = default;
Selector::~Selector() = default;

namespace {
int FindItemIndexForValue(const std::vector<std::any>& source, const std::any& value)
{
    const auto it = std::find_if(source.begin(), source.end(), [&value](const std::any& candidate) {
        return NeoWPF::UI::Properties::ValuesEqual(candidate, value);
    });
    return it == source.end() ? -1 : static_cast<int>(std::distance(source.begin(), it));
}

int FindItemIndexForVisual(const std::vector<std::shared_ptr<FrameworkElement>>& items, const std::any& value)
{
    const auto it = std::find_if(items.begin(), items.end(), [&value](const std::shared_ptr<FrameworkElement>& candidate) {
        return candidate && NeoWPF::UI::Properties::ValuesEqual(candidate->GetDataContext(), value);
    });
    return it == items.end() ? -1 : static_cast<int>(std::distance(items.begin(), it));
}
}

int Selector::GetSelectedIndex() const noexcept
{
    try {
        return GetValue<int>(SelectedIndexProperty());
    } catch (const std::bad_any_cast&) {
        return -1;
    }
}

void Selector::SetSelectedIndex(int index)
{
    const auto oldIndex = GetSelectedIndex();
    std::any oldItem;

    if (oldIndex >= 0) {
        if (oldIndex < static_cast<int>(GetItemsSource().size())) {
            oldItem = GetItemsSource()[oldIndex];
        } else if (oldIndex < static_cast<int>(GetItems().size())) {
            const auto& item = GetItems()[oldIndex];
            if (item) {
                oldItem = item->GetDataContext();
            }
        }
    }

    if (index >= 0 && index < static_cast<int>(GetItemsSource().size())) {
        SetSelectedItem(GetItemsSource()[index]);
        return;
    }

    if (index >= 0 && index < static_cast<int>(GetItems().size())) {
        const auto& item = GetItems()[index];
        if (item) {
            SetSelectedItem(item->GetDataContext());
            return;
        }
    }

    SetSelectedItem(std::any{});
    OnSelectionChanged(oldIndex, index, oldItem, GetSelectedItem());
}

std::any Selector::GetSelectedItem() const noexcept
{
    if (m_selectedItems.empty()) {
        return std::any{};
    }
    return m_selectedItems.back();
}

void Selector::UpdateSelectionState(const std::any& item)
{
    if (GetSelectionMode() == SelectionMode::Multiple) {
        if (!item.has_value()) {
            m_selectedItems.clear();
            return;
        }

        const auto selectedIt = std::find_if(m_selectedItems.begin(), m_selectedItems.end(), [&item](const std::any& candidate) {
            return NeoWPF::UI::Properties::ValuesEqual(candidate, item);
        });

        if (selectedIt != m_selectedItems.end()) {
            m_selectedItems.erase(selectedIt);
        } else {
            m_selectedItems.push_back(item);
        }
        return;
    }

    m_selectedItems.clear();
    if (item.has_value()) {
        m_selectedItems.push_back(item);
    }
}

void Selector::SyncSelectedIndexFromState()
{
    if (m_selectedItems.empty()) {
        SetValue(SelectedIndexProperty(), -1);
        return;
    }

    const auto selectedValue = m_selectedItems.back();
    const int sourceIndex = FindItemIndexForValue(GetItemsSource(), selectedValue);
    if (sourceIndex >= 0) {
        SetValue(SelectedIndexProperty(), sourceIndex);
    } else {
        SetValue(SelectedIndexProperty(), FindItemIndexForVisual(GetItems(), selectedValue));
    }
}

void Selector::SetSelectedItem(const std::any& item)
{
    if (IsApplyingPropertyValue()) {
        return;
    }

    const auto oldItem = GetSelectedItem();
    const auto oldIndex = GetSelectedIndex();

    UpdateSelectionState(item);

    const auto selectedValue = m_selectedItems.empty() ? std::any{} : m_selectedItems.back();
    SetValue(SelectedItemProperty(), selectedValue);
    SyncSelectedIndexFromState();

    OnSelectionChanged(oldIndex, GetSelectedIndex(), oldItem, selectedValue);
}

SelectionMode Selector::GetSelectionMode() const noexcept
{
    try {
        return GetValue<SelectionMode>(SelectionModeProperty());
    } catch (const std::bad_any_cast&) {
        return SelectionMode::Single;
    }
}

void Selector::SetSelectionMode(SelectionMode mode) noexcept
{
    SetValue(SelectionModeProperty(), mode);
}

bool Selector::IsSelected(const std::any& item) const
{
    if (GetSelectionMode() == SelectionMode::Multiple) {
        return std::any_of(m_selectedItems.begin(), m_selectedItems.end(), [&item](const std::any& candidate) {
            return NeoWPF::UI::Properties::ValuesEqual(candidate, item);
        });
    }

    return NeoWPF::UI::Properties::ValuesEqual(GetSelectedItem(), item);
}

void Selector::OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem)
{
    (void)oldIndex;
    (void)newIndex;
    (void)oldItem;
    (void)newItem;
}

}

