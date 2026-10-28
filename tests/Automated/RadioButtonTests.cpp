#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Controls/RadioButton/RadioButton.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

#include <memory>

namespace {

void RaisePointerEvent(NeoWPF::UI::RadioButton& radioButton,
                       const NeoWPF::Input::Core::RoutedEvent& routedEvent,
                       NeoWPF::Input::Core::PointerPhase phase,
                       std::uint32_t changedButton = 0,
                       std::uint32_t buttons = 0)
{
    const NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse, phase,
        10.0, 10.0, buttons, changedButton);
    NeoWPF::Input::Core::RoutedPointerEventArgs args(routedEvent, packet);
    args.SetSource(&radioButton);
    args.SetOriginalSource(&radioButton);
    radioButton.RaiseEvent(&args);
}

class RadioButtonTests : public ::testing::Test {
protected:
    void SetUp() override { NeoWPF::Core::Application::Initialize(); }
    void TearDown() override { NeoWPF::Core::Application::Shutdown(); }
};

std::shared_ptr<NeoWPF::UI::RadioButton> MakeRadio(const std::wstring& groupName)
{
    auto radio = std::make_shared<NeoWPF::UI::RadioButton>();
    radio->SetGroupName(groupName);
    return radio;
}

}

TEST_F(RadioButtonTests, SameGroupSelectionIsMutuallyExclusive)
{
    auto scope = std::make_shared<NeoWPF::UI::Visual>();
    auto first = MakeRadio(L"Modes");
    auto second = MakeRadio(L"Modes");
    scope->AddChild(first);
    scope->AddChild(second);

    first->SetChecked(true);
    second->SetChecked(true);

    EXPECT_FALSE(first->IsChecked());
    EXPECT_TRUE(second->IsChecked());
}

TEST_F(RadioButtonTests, DifferentGroupsRemainIndependent)
{
    auto scope = std::make_shared<NeoWPF::UI::Visual>();
    auto first = MakeRadio(L"First");
    auto second = MakeRadio(L"Second");
    scope->AddChild(first);
    scope->AddChild(second);

    first->SetChecked(true);
    second->SetChecked(true);

    EXPECT_TRUE(first->IsChecked());
    EXPECT_TRUE(second->IsChecked());
}

TEST_F(RadioButtonTests, DirectClearingIsAllowed)
{
    auto radio = std::make_shared<NeoWPF::UI::RadioButton>();
    radio->SetChecked(true);
    radio->SetChecked(false);
    EXPECT_FALSE(radio->IsChecked());
}

TEST_F(RadioButtonTests, PrimaryPointerReleaseSelectsAndRaisesClick)
{
    NeoWPF::UI::RadioButton radioButton;
    int clickCount = 0;
    radioButton.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_TRUE(radioButton.IsChecked());
    EXPECT_EQ(clickCount, 1);
}

TEST_F(RadioButtonTests, PointerActivationDoesNotUncheckAlreadySelectedButton)
{
    NeoWPF::UI::RadioButton radioButton;
    radioButton.SetChecked(true);
    int clickCount = 0;
    radioButton.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_TRUE(radioButton.IsChecked());
    EXPECT_EQ(clickCount, 1);
}

TEST_F(RadioButtonTests, PointerReleaseWithoutPressDoesNotActivate)
{
    NeoWPF::UI::RadioButton radioButton;
    int clickCount = 0;
    radioButton.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_FALSE(radioButton.IsChecked());
    EXPECT_EQ(clickCount, 0);
}

TEST_F(RadioButtonTests, NonPrimaryReleaseCancelsPendingActivation)
{
    NeoWPF::UI::RadioButton radioButton;
    int clickCount = 0;
    radioButton.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x02u, 0x01u);
    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_FALSE(radioButton.IsChecked());
    EXPECT_EQ(clickCount, 0);
}

TEST_F(RadioButtonTests, DisabledPointerActivationDoesNotChangeStateOrRaiseClick)
{
    NeoWPF::UI::RadioButton radioButton;
    radioButton.SetEnabled(false);
    int clickCount = 0;
    radioButton.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_FALSE(radioButton.IsChecked());
    EXPECT_EQ(clickCount, 0);
}

TEST_F(RadioButtonTests, PointerCancelPreventsLaterReleaseActivation)
{
    NeoWPF::UI::RadioButton radioButton;
    int clickCount = 0;
    radioButton.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerCanceledEvent(),
                      NeoWPF::Input::Core::PointerPhase::Cancel);
    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_FALSE(radioButton.IsChecked());
    EXPECT_EQ(clickCount, 0);
}

TEST_F(RadioButtonTests, PointerExitPreventsLaterReleaseActivation)
{
    NeoWPF::UI::RadioButton radioButton;
    int clickCount = 0;
    radioButton.Click += [&clickCount] { ++clickCount; };

    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0x01u, 0x01u);
    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerExitedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Leave);
    RaisePointerEvent(radioButton, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);

    EXPECT_FALSE(radioButton.IsChecked());
    EXPECT_EQ(clickCount, 0);
}

TEST_F(RadioButtonTests, XamlRegistrationAndPropertiesAreAvailable)
{
    const auto* type = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find("RadioButton");
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->CppTypeName, "NeoWPF::UI::RadioButton");
    EXPECT_EQ(type->HeaderPath, "NeoWPF/UI/Controls/RadioButton/RadioButton.h");

    const auto root = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<RadioButton Text="Choice" GroupName="Modes" IsChecked="True" FontSize="16" FontFamily="Arial" />)");
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(root);
    ASSERT_NE(tree, nullptr);
    const auto radio = dynamic_cast<const NeoWPF::UI::RadioButton*>(tree.get());
    ASSERT_NE(radio, nullptr);
    EXPECT_EQ(radio->GetText(), L"Choice");
    EXPECT_EQ(radio->GetGroupName(), L"Modes");
    EXPECT_TRUE(radio->IsChecked());
    EXPECT_EQ(radio->GetFontSize(), 16);
    EXPECT_EQ(radio->GetFontFamily(), L"Arial");
}
