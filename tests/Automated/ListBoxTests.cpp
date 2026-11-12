#include <gtest/gtest.h>

#include <algorithm>

#if defined(_WIN32)
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Window.h>
#endif
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/ListBox/ListBox.h>

namespace {

struct SelectorItemModel {
    int id = 0;
};

TEST(ListBoxItemTests, IsSelectedDefaultsToFalse)
{
    NeoWPF::UI::ListBoxItem item;

    EXPECT_FALSE(item.GetIsSelected());
    EXPECT_FALSE(item.GetValue<bool>(NeoWPF::UI::ListBoxItem::IsSelectedProperty()));
}

TEST(ListBoxItemTests, StringContentBecomesTextBlockContent)
{
    NeoWPF::UI::ListBoxItem item;

    item.SetValue(NeoWPF::UI::Content::ContentControl::ContentProperty(),
                  std::string("Alpha"));

    const auto content = item.GetContentElement();
    ASSERT_NE(content, nullptr);
    const auto text = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(content);
    ASSERT_NE(text, nullptr);
    EXPECT_EQ(text->GetText(), std::wstring(L"Alpha"));
}

TEST(ListBoxItemTests, WideCStringContentBecomesTextBlockContent)
{
    NeoWPF::UI::ListBoxItem item;

    item.SetValue(NeoWPF::UI::Content::ContentControl::ContentProperty(), L"Alpha");

    const auto content = item.GetContentElement();
    ASSERT_NE(content, nullptr);
    const auto text = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(content);
    ASSERT_NE(text, nullptr);
    EXPECT_EQ(text->GetText(), std::wstring(L"Alpha"));
}

TEST(ListBoxItemTests, StringContentIsExposedByContentPresenter)
{
    NeoWPF::UI::ListBoxItem item;

    item.SetValue(NeoWPF::UI::Content::ContentControl::ContentProperty(),
                  std::string("Bravo"));

    ASSERT_NE(item.GetPresenter(), nullptr);
    const auto content = item.GetPresenter()->GetContentElement();
    ASSERT_NE(content, nullptr);
    const auto text = std::dynamic_pointer_cast<NeoWPF::UI::TextBlock>(content);
    ASSERT_NE(text, nullptr);
    EXPECT_EQ(text->GetText(), std::wstring(L"Bravo"));
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
    EXPECT_EQ(selectedCommands.commands[0].GetKind(),
              NeoWPF::Rendering::CommandList::CommandKind::RoundedRect);
    EXPECT_EQ(selectedCommands.commands[1].GetKind(),
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

TEST(ListBoxTests, RoutedPointerReleaseSelectsTheItem)
{
    NeoWPF::UI::ListBox listBox;
    auto clickedItem = std::make_shared<NeoWPF::UI::ListBoxItem>();
    clickedItem->SetDataContext(std::string("clicked"));

    listBox.AddItem(clickedItem);

    NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse,
        NeoWPF::Input::Core::PointerPhase::Up, 0.0, 0.0, 0, 0x01u);
    NeoWPF::Input::Core::RoutedPointerEventArgs args(
        NeoWPF::Input::Core::PointerReleasedEvent(), packet);
    args.SetSource(clickedItem.get());
    args.SetOriginalSource(clickedItem.get());
    clickedItem->RaiseEvent(&args);

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

TEST(ListBoxTests, ObjectValuedItemsUseSharedIdentityForSelection)
{
    NeoWPF::UI::ListBox listBox;
    listBox.SetSelectionMode(NeoWPF::UI::SelectionMode::Multiple);

    auto model = std::make_shared<SelectorItemModel>();
    auto item = std::make_shared<NeoWPF::UI::ListBoxItem>();
    item->SetDataContext(model);
    listBox.AddItem(item);

    listBox.SetSelectedItem(model);

    EXPECT_TRUE(listBox.IsSelected(model));
    EXPECT_EQ(listBox.GetSelectedIndex(), 0);
    EXPECT_TRUE(item->GetIsSelected());

    listBox.SetSelectedItem(model);

    EXPECT_FALSE(listBox.IsSelected(model));
    EXPECT_EQ(listBox.GetSelectedIndex(), -1);
    EXPECT_FALSE(item->GetIsSelected());
}

TEST(ListBoxTests, TypedObjectItemsSourceSupportsSelectionByIndexAndValue)
{
    NeoWPF::UI::ListBox listBox;
    listBox.SetSelectionMode(NeoWPF::UI::SelectionMode::Multiple);

    auto model = std::make_shared<SelectorItemModel>();
    listBox.SetItemsSource(std::vector<std::shared_ptr<SelectorItemModel>>{model});
    listBox.SetItemTemplate([](const std::any&, int) {
        return std::make_shared<NeoWPF::UI::ListBoxItem>();
    });

    ASSERT_EQ(listBox.GetVisualChildren().size(), 1u);
    auto item = dynamic_cast<NeoWPF::UI::ListBoxItem*>(listBox.GetVisualChildren()[0]);
    ASSERT_NE(item, nullptr);

    listBox.SetSelectedIndex(0);
    EXPECT_TRUE(listBox.IsSelected(model));
    EXPECT_TRUE(item->GetIsSelected());

    listBox.SetSelectedItem(model);
    EXPECT_FALSE(listBox.IsSelected(model));
    EXPECT_EQ(listBox.GetSelectedIndex(), -1);
    EXPECT_FALSE(item->GetIsSelected());
}

}
