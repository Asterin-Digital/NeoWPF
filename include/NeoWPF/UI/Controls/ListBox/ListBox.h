#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Controls/Selector/Selector.h>

namespace NeoWPF::UI {

class ListBox : public Selector {
public:
    ListBox();
    ~ListBox() override;

    void AddItem(std::shared_ptr<FrameworkElement> item) override;
    void SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items);
    virtual void SyncSelection(const std::any& item);

    std::shared_ptr<FrameworkElement> GetContainerForItem(const std::any& data, int index) override;
    void PrepareContainer(const std::shared_ptr<FrameworkElement>& container,
                          const std::any& data,
                          int index) override;

    void OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem) override;

private:
    void AttachSelectionHandler(const std::shared_ptr<FrameworkElement>& item);
};

/*
    @summary
    Hosts one item in a ListBox.
*/
class ListBoxItem : public Content::ContentControl {
public:
    static const Properties::DependencyProperty& IsSelectedProperty();

    ListBoxItem();
    ~ListBoxItem() override;

    void SetOwner(ListBox* owner) noexcept;
    ListBox* GetOwner() const noexcept;

    void SetIsSelected(bool selected) noexcept;
    bool GetIsSelected() const noexcept;

    void OnMouseUp(int x, int y, std::uint32_t buttonState) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

protected:
    virtual void OnSelectionStateChanged() {}

private:
    ListBox* m_owner = nullptr;
};

}
