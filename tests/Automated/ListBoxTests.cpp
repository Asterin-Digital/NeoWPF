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

#if defined(_WIN32)

class ListBoxRenderCommandTest : public ::testing::Test {
protected:
    static void SetUpTestSuite()
    {
        NeoWPF::Core::Application::Initialize();
        testListBox = std::make_shared<NeoWPF::UI::ListBox>();
        testWindowHost = std::make_shared<NeoWPF::UI::Window>();
        testWindowHost->SetTitle(L"NeoWPF ListBox test");
        testWindowHost->SetContent(testListBox);
        testWindow = NeoWPF::Core::Application::Current().CreateWindow(
            testWindowHost, 320, 240);
        ASSERT_NE(testWindow.Handle(), nullptr);

        testWindow.Hide();
    }

    static void TearDownTestSuite()
    {
        testWindow.Close();
        NeoWPF::Core::Application::Shutdown();
    }

    static NeoWPF::Platform::Window testWindow;
    static std::shared_ptr<NeoWPF::UI::Window> testWindowHost;
    static std::shared_ptr<NeoWPF::UI::ListBox> testListBox;
};

NeoWPF::Platform::Window ListBoxRenderCommandTest::testWindow;
std::shared_ptr<NeoWPF::UI::Window> ListBoxRenderCommandTest::testWindowHost;
std::shared_ptr<NeoWPF::UI::ListBox> ListBoxRenderCommandTest::testListBox;

#endif

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

#if defined(_WIN32)
TEST_F(ListBoxRenderCommandTest, StringContentProducesRenderCommandsAfterLayout)
{
    auto& listBox = *ListBoxRenderCommandTest::testListBox;
    auto item = std::make_shared<NeoWPF::UI::ListBoxItem>();
    item->SetValue(NeoWPF::UI::Content::ContentControl::ContentProperty(),
                   std::string("Alpha"));
    listBox.AddItem(item);

    listBox.Measure(220, 180);
    listBox.Arrange(220, 180);

    ASSERT_EQ(item->GetParent(), &listBox);
    ASSERT_NE(item->GetPresenter(), nullptr);
    ASSERT_EQ(item->GetPresenter()->GetParent(), item.get());
    ASSERT_EQ(item->GetPresenter()->GetContentElement()->GetParent(), item->GetPresenter().get());

    NeoWPF::Rendering::CommandList commands;
    listBox.BuildRenderCommands(commands);

    const auto textCommand = std::find_if(
        commands.commands.begin(),
        commands.commands.end(),
        [](const auto& command) {
            return command.kind == NeoWPF::Rendering::CommandList::CommandKind::TexturedQuad;
        });
    EXPECT_NE(textCommand, commands.commands.end());
}
#endif

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
