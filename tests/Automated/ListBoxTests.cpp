#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Renderer.h>
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

}
