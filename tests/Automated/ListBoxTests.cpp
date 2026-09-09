#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Controls/ListBox/ListBox.h>

namespace {

TEST(ListBoxItemTests, IsSelectedDefaultsToFalse)
{
    NeoWPF::UI::ListBoxItem item;

    EXPECT_FALSE(item.GetIsSelected());
    EXPECT_FALSE(item.GetValue<bool>(NeoWPF::UI::ListBoxItem::IsSelectedProperty()));
}

TEST(ListBoxItemTests, IsSelectedSetterUpdatesDependencyProperty)
{
    NeoWPF::UI::ListBoxItem item;

    item.SetIsSelected(true);

    EXPECT_TRUE(item.GetIsSelected());
    EXPECT_TRUE(item.GetValue<bool>(NeoWPF::UI::ListBoxItem::IsSelectedProperty()));

    item.SetIsSelected(false);

    EXPECT_FALSE(item.GetIsSelected());
}

TEST(ListBoxItemTests, DependencyPropertyWriteUpdatesIsSelected)
{
    NeoWPF::UI::ListBoxItem item;
    const auto& property = NeoWPF::UI::ListBoxItem::IsSelectedProperty();

    item.SetValue(property, true);
    EXPECT_TRUE(item.GetIsSelected());

    item.SetValue(property, false);
    EXPECT_FALSE(item.GetIsSelected());
}

TEST(ListBoxItemTests, SelectedStateAddsSelectionRenderCommands)
{
    NeoWPF::UI::ListBoxItem item;
    item.Arrange(100, 24);

    NeoWPF::Rendering::CommandList unselectedCommands;
    item.BuildRenderCommands(unselectedCommands);
    EXPECT_TRUE(unselectedCommands.commands.empty());

    item.SetIsSelected(true);

    NeoWPF::Rendering::CommandList selectedCommands;
    item.BuildRenderCommands(selectedCommands);
    ASSERT_EQ(selectedCommands.commands.size(), 2u);
    EXPECT_EQ(selectedCommands.commands[0].kind,
              NeoWPF::Rendering::CommandList::CommandKind::RoundedRect);
    EXPECT_EQ(selectedCommands.commands[1].kind,
              NeoWPF::Rendering::CommandList::CommandKind::RoundedRect);
}

TEST(ListBoxTests, SelectionSyncsExistingContainerItems)
{
    NeoWPF::UI::ListBox listBox;
    auto first = std::make_shared<NeoWPF::UI::ListBoxItem>();
    auto second = std::make_shared<NeoWPF::UI::ListBoxItem>();

    first->SetOwner(&listBox);
    second->SetOwner(&listBox);
    first->SetDataContext(std::string("first"));
    second->SetDataContext(std::string("second"));

    listBox.AddItem(first);
    listBox.AddItem(second);

    listBox.SetSelectedItem(std::string("second"));

    EXPECT_FALSE(first->GetIsSelected());
    EXPECT_TRUE(second->GetIsSelected());

    listBox.SetSelectedIndex(0);

    EXPECT_TRUE(first->GetIsSelected());
    EXPECT_FALSE(second->GetIsSelected());
}

TEST(ListBoxTests, RoutedMouseUpEventSelectsTheItem)
{
    NeoWPF::UI::ListBox listBox;
    auto clickedItem = std::make_shared<NeoWPF::UI::ListBoxItem>();
    clickedItem->SetDataContext(std::string("clicked"));

    listBox.AddItem(clickedItem);

    NeoWPF::Input::InputManager::GetInstance().SetRootElement(&listBox);
    NeoWPF::Input::InputManager::GetInstance().ClickAt(clickedItem.get(), 0, 0);

    EXPECT_TRUE(listBox.GetSelectedItem().has_value());
    EXPECT_TRUE(NeoWPF::UI::Properties::ValuesEqual(listBox.GetSelectedItem(), std::string("clicked")));
    EXPECT_TRUE(clickedItem->GetIsSelected());
}

TEST(ListBoxTests, SingleSelectionModeKeepsOnlyOneItemSelected)
{
    NeoWPF::UI::ListBox listBox;
    listBox.SetSelectionMode(NeoWPF::UI::SelectionMode::Single);

    auto first = std::make_shared<NeoWPF::UI::ListBoxItem>();
    auto second = std::make_shared<NeoWPF::UI::ListBoxItem>();
    first->SetDataContext(std::string("first"));
    second->SetDataContext(std::string("second"));

    listBox.AddItem(first);
    listBox.AddItem(second);

    listBox.SetSelectedItem(std::string("first"));
    listBox.SetSelectedItem(std::string("second"));

    EXPECT_FALSE(first->GetIsSelected());
    EXPECT_TRUE(second->GetIsSelected());
    EXPECT_TRUE(NeoWPF::UI::Properties::ValuesEqual(listBox.GetSelectedItem(), std::string("second")));
}

TEST(ListBoxTests, MultipleSelectionModeTogglesItems)
{
    NeoWPF::UI::ListBox listBox;
    listBox.SetSelectionMode(NeoWPF::UI::SelectionMode::Multiple);

    auto first = std::make_shared<NeoWPF::UI::ListBoxItem>();
    auto second = std::make_shared<NeoWPF::UI::ListBoxItem>();
    first->SetDataContext(std::string("first"));
    second->SetDataContext(std::string("second"));

    listBox.AddItem(first);
    listBox.AddItem(second);

    listBox.SetSelectedItem(std::string("first"));
    EXPECT_TRUE(first->GetIsSelected());
    EXPECT_FALSE(second->GetIsSelected());

    listBox.SetSelectedItem(std::string("second"));
    EXPECT_TRUE(first->GetIsSelected());
    EXPECT_TRUE(second->GetIsSelected());

    listBox.SetSelectedItem(std::string("first"));
    EXPECT_FALSE(first->GetIsSelected());
    EXPECT_TRUE(second->GetIsSelected());
}

}
