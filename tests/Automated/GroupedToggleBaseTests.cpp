#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Controls/Base/GroupedToggleBase/GroupedToggleBase.h>
#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

#include <memory>

namespace {

using NeoWPF::UI::GroupedToggleBase;
using NeoWPF::UI::Visual;

class GroupedToggleBaseTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        NeoWPF::Core::Application::Initialize();
    }

    void TearDown() override
    {
        NeoWPF::Core::Application::Shutdown();
    }
};

std::shared_ptr<GroupedToggleBase> MakeToggle(const std::wstring& groupName)
{
    auto toggle = std::make_shared<GroupedToggleBase>();
    toggle->SetGroupName(groupName);
    return toggle;
}

}

TEST_F(GroupedToggleBaseTests, CheckingOnePeerUnchecksThePreviouslyCheckedPeer)
{
    auto scope = std::make_shared<Visual>();
    auto first = MakeToggle(L"Modes");
    auto second = MakeToggle(L"Modes");
    scope->AddChild(first);
    scope->AddChild(second);

    first->SetChecked(true);
    second->SetChecked(true);

    EXPECT_FALSE(first->IsChecked());
    EXPECT_TRUE(second->IsChecked());
}

TEST_F(GroupedToggleBaseTests, EmptyGroupNameUsesTheImplicitGroup)
{
    auto scope = std::make_shared<Visual>();
    auto first = MakeToggle(L"");
    auto second = MakeToggle(L"");
    scope->AddChild(first);
    scope->AddChild(second);

    first->SetChecked(true);
    second->SetChecked(true);

    EXPECT_FALSE(first->IsChecked());
    EXPECT_TRUE(second->IsChecked());
}

TEST_F(GroupedToggleBaseTests, DifferentGroupNamesDoNotInteract)
{
    auto scope = std::make_shared<Visual>();
    auto first = MakeToggle(L"First");
    auto second = MakeToggle(L"Second");
    scope->AddChild(first);
    scope->AddChild(second);

    first->SetChecked(true);
    second->SetChecked(true);

    EXPECT_TRUE(first->IsChecked());
    EXPECT_TRUE(second->IsChecked());
}

TEST_F(GroupedToggleBaseTests, NestedScopesRemainIndependent)
{
    auto root = std::make_shared<Visual>();
    auto nestedScope = std::make_shared<Visual>();
    auto outer = MakeToggle(L"Modes");
    auto inner = MakeToggle(L"Modes");
    root->AddChild(outer);
    root->AddChild(nestedScope);
    nestedScope->AddChild(inner);

    outer->SetChecked(true);
    inner->SetChecked(true);

    EXPECT_TRUE(outer->IsChecked());
    EXPECT_TRUE(inner->IsChecked());
}

TEST_F(GroupedToggleBaseTests, RepeatedSelectionDoesNotRaiseAnExtraUncheck)
{
    auto scope = std::make_shared<Visual>();
    auto first = MakeToggle(L"Modes");
    auto second = MakeToggle(L"Modes");
    scope->AddChild(first);
    scope->AddChild(second);

    int firstUnchecked = 0;
    first->Unchecked += [&firstUnchecked] { ++firstUnchecked; };

    first->SetChecked(true);
    second->SetChecked(true);
    second->SetChecked(true);

    EXPECT_EQ(firstUnchecked, 1);
    EXPECT_FALSE(first->IsChecked());
    EXPECT_TRUE(second->IsChecked());
}

TEST_F(GroupedToggleBaseTests, DetachedTogglesDoNotReachIntoAnotherScope)
{
    auto firstScope = std::make_shared<Visual>();
    auto secondScope = std::make_shared<Visual>();
    auto first = MakeToggle(L"Modes");
    auto second = MakeToggle(L"Modes");
    firstScope->AddChild(first);
    secondScope->AddChild(second);

    first->SetChecked(true);
    second->SetChecked(true);

    EXPECT_TRUE(first->IsChecked());
    EXPECT_TRUE(second->IsChecked());
}

TEST_F(GroupedToggleBaseTests, BasePropertiesExposeStaticHandlesAndSynchronizeDirectSetters)
{
    NeoWPF::UI::ToggleBase toggle;
    toggle.SetChecked(true);
    toggle.SetContentText(L"Enabled");

    EXPECT_TRUE(toggle.GetValue<bool>(NeoWPF::UI::ToggleBase::IsCheckedProperty()));
    EXPECT_EQ(toggle.GetValue<std::wstring>(NeoWPF::UI::ToggleBase::ContentTextProperty()), L"Enabled");

    NeoWPF::UI::GroupedToggleBase groupedToggle;
    groupedToggle.SetGroupName(L"Modes");
    EXPECT_EQ(groupedToggle.GetValue<std::wstring>(NeoWPF::UI::GroupedToggleBase::GroupNameProperty()), L"Modes");
}

TEST_F(GroupedToggleBaseTests, RangePropertiesClampAndExposeTheCanonicalValues)
{
    NeoWPF::UI::RangeBase range;
    range.SetMinimum(10.0);
    range.SetMaximum(20.0);
    range.SetValue(15.0);
    range.SetSmallChange(2.0);
    range.SetLargeChange(5.0);

    EXPECT_DOUBLE_EQ(range.GetValue(), 15.0);
    const auto& dependencyObject = static_cast<const NeoWPF::UI::Properties::DependencyObject&>(range);
    EXPECT_DOUBLE_EQ(dependencyObject.GetValue<double>(NeoWPF::UI::RangeBase::ValueProperty()), 15.0);
    EXPECT_DOUBLE_EQ(dependencyObject.GetValue<double>(NeoWPF::UI::RangeBase::MinimumProperty()), 10.0);
    EXPECT_DOUBLE_EQ(dependencyObject.GetValue<double>(NeoWPF::UI::RangeBase::MaximumProperty()), 20.0);
    EXPECT_DOUBLE_EQ(dependencyObject.GetValue<double>(NeoWPF::UI::RangeBase::SmallChangeProperty()), 2.0);
    EXPECT_DOUBLE_EQ(dependencyObject.GetValue<double>(NeoWPF::UI::RangeBase::LargeChangeProperty()), 5.0);

    range.SetValue(100.0);
    EXPECT_DOUBLE_EQ(range.GetValue(), 20.0);
    EXPECT_DOUBLE_EQ(dependencyObject.GetValue<double>(NeoWPF::UI::RangeBase::ValueProperty()), 20.0);

    range.SetValue(15.0);
    range.SetMinimum(18.0);
    EXPECT_DOUBLE_EQ(range.GetValue(), 18.0);
    EXPECT_DOUBLE_EQ(dependencyObject.GetValue<double>(NeoWPF::UI::RangeBase::ValueProperty()), 18.0);
}

TEST_F(GroupedToggleBaseTests, BasePropertiesAreInheritedByDerivedControls)
{
    NeoWPF::UI::ButtonBase button;
    button.SetCommandParameter(std::any(42));
    button.SetCommandTarget(&button);

    const auto commandParameter = button.GetRawValue(NeoWPF::UI::ButtonBase::CommandParameterProperty());
    EXPECT_TRUE(commandParameter.has_value());
    EXPECT_EQ(std::any_cast<int>(commandParameter), 42);
    EXPECT_EQ(button.GetValue<NeoWPF::UI::UIElement*>(NeoWPF::UI::ButtonBase::CommandTargetProperty()), &button);
}
