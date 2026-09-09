#pragma once

#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <any>
#include <vector>

namespace NeoWPF::UI {

enum class SelectionMode {
    Single,
    Multiple,
    Extended
};

class Selector : public ItemsControl {
public:
    static const Properties::DependencyProperty& SelectedIndexProperty();
    static const Properties::DependencyProperty& SelectedItemProperty();
    static const Properties::DependencyProperty& SelectionModeProperty();

    Selector();
    ~Selector() override;

    int GetSelectedIndex() const noexcept;
    void SetSelectedIndex(int index);

    std::any GetSelectedItem() const noexcept;
    void SetSelectedItem(const std::any& item);

    SelectionMode GetSelectionMode() const noexcept;
    void SetSelectionMode(SelectionMode mode) noexcept;

    bool IsSelected(const std::any& item) const;

protected:
    virtual void OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem);

private:
    void UpdateSelectionState(const std::any& item);
    void SyncSelectedIndexFromState();

    std::vector<std::any> m_selectedItems;
};

}
