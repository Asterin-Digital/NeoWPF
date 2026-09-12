#pragma once

#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/UI/Controls/Selector/Selector.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <memory>

namespace NeoWPF::UI {

class ListBox;

class ComboBoxItemTemplate;
class ComboBoxTemplate;

class ComboBoxItem : public ListBoxItem {
public:
    static const Properties::DependencyProperty& IsHighlightedProperty();

    ComboBoxItem();
    ~ComboBoxItem() override;

    void SetIsHighlighted(bool highlighted) noexcept;
    bool GetIsHighlighted() const noexcept;

    void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) override;
    void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) override;

private:
    void OnSelectionStateChanged() override;
    void ApplyVisualStates(bool useTransitions);
};

class ComboBoxItemTemplate : public Templates::ControlTemplate {
public:
    ComboBoxItemTemplate();
};

class ComboBoxTemplate : public Templates::ControlTemplate {
public:
    ComboBoxTemplate();
};

class ComboBox : public Selector {
public:
    static const Properties::DependencyProperty& IsDropDownOpenProperty();
    static const Properties::DependencyProperty& PlacementProperty();
    static const Properties::DependencyProperty& SelectionBoxItemProperty();

    ComboBox();
    ~ComboBox() override;

    bool GetIsDropDownOpen() const noexcept;
    void SetIsDropDownOpen(bool isOpen);

    PopupPlacement GetPlacement() const noexcept;
    void SetPlacement(PopupPlacement placement);

    std::any GetSelectionBoxItem() const noexcept;

    void SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items);
    void AddItem(std::shared_ptr<FrameworkElement> item) override;
    void ClearItems();
    void SetItemsSource(const std::vector<std::any>& source);
    void SetItemTemplate(const Controls::ItemsControl::Virtualization::ItemTemplate& templ);
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

protected:
    void OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem) override;

private:
    void UpdateDisplayedItem(const std::any& item);

    std::shared_ptr<Content::ContentPresenter> m_display;
    std::shared_ptr<Popup> m_popup;
    std::shared_ptr<ListBox> m_dropDown;
};

}
