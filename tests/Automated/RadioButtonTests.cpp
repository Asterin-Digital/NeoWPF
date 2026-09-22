#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/RadioButton/RadioButton.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

#include <memory>

namespace {

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
