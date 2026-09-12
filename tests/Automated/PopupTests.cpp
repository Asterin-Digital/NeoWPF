#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

#include <any>
#include <memory>
#include <string>

namespace {

class PopupTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        NeoWPF::Core::Application::Initialize();
    }

    void TearDown() override
    {
        NeoWPF::Input::InputManager::GetInstance().SetOverlayElements({});
        NeoWPF::Input::InputManager::GetInstance().SetRootElement(nullptr);
        NeoWPF::Core::Application::Shutdown();
    }
};

TEST_F(PopupTests, DefaultsMatchGenericWpfPopupPolicy)
{
    NeoWPF::UI::Popup popup;

    EXPECT_FALSE(popup.GetIsOpen());
    EXPECT_TRUE(popup.GetStaysOpen());
    EXPECT_EQ(popup.GetPlacement(), NeoWPF::UI::PopupPlacement::Bottom);
    EXPECT_EQ(popup.GetHorizontalOffset(), 0);
    EXPECT_EQ(popup.GetVerticalOffset(), 0);
    EXPECT_FALSE(popup.IsHitTestVisible());
}

TEST_F(PopupTests, DependencyPropertiesStaySynchronizedWithPublicApi)
{
    NeoWPF::UI::Popup popup;

    popup.SetStaysOpen(false);
    popup.SetPlacement(NeoWPF::UI::PopupPlacement::Right);
    popup.SetHorizontalOffset(13);
    popup.SetVerticalOffset(-7);

    EXPECT_FALSE(popup.GetValue<bool>(NeoWPF::UI::Popup::StaysOpenProperty()));
    EXPECT_EQ(popup.GetValue<NeoWPF::UI::PopupPlacement>(NeoWPF::UI::Popup::PlacementProperty()),
              NeoWPF::UI::PopupPlacement::Right);
    EXPECT_EQ(popup.GetValue<int>(NeoWPF::UI::Popup::HorizontalOffsetProperty()), 13);
    EXPECT_EQ(popup.GetValue<int>(NeoWPF::UI::Popup::VerticalOffsetProperty()), -7);
}

TEST_F(PopupTests, OpenAndCloseControlRenderingAndHitTesting)
{
    auto popup = std::make_shared<NeoWPF::UI::Popup>();
    auto button = std::make_shared<NeoWPF::UI::Button>(L"Popup action");
    popup->SetContent(button);
    popup->Arrange(160, 40);

    NeoWPF::Rendering::CommandList closedCommands;
    popup->BuildRenderCommands(closedCommands);
    EXPECT_TRUE(closedCommands.commands.empty());
    EXPECT_FALSE(popup->IsHitTestVisible());

    popup->SetIsOpen(true);
    popup->Arrange(160, 40);

    NeoWPF::Rendering::CommandList openCommands;
    popup->BuildRenderCommands(openCommands);
    EXPECT_FALSE(openCommands.commands.empty());
    EXPECT_TRUE(popup->IsHitTestVisible());

    popup->SetIsOpen(false);
    NeoWPF::Rendering::CommandList closedAgainCommands;
    popup->BuildRenderCommands(closedAgainCommands);
    EXPECT_TRUE(closedAgainCommands.commands.empty());
    EXPECT_FALSE(popup->IsHitTestVisible());
}

TEST_F(PopupTests, PlacementAndOffsetsUseTargetBounds)
{
    auto window = std::make_shared<NeoWPF::UI::Window>();
    auto root = std::make_shared<NeoWPF::UI::StackPanel>();
    auto target = std::make_shared<NeoWPF::UI::Button>(L"Target");
    window->SetContent(root);
    root->AddChild(target);
    root->Arrange(500, 400);
    target->Arrange(100, 30);

    NeoWPF::UI::Popup popup;
    popup.SetPlacementTarget(target.get());
    popup.SetHorizontalOffset(5);
    popup.SetVerticalOffset(9);
    popup.SetIsOpen(true);
    popup.Arrange(80, 20);

    int targetX = 0;
    int targetY = 0;
    target->GetAbsolutePosition(targetX, targetY);
    EXPECT_EQ(popup.GetLayoutOffsetX(), targetX + 5);
    EXPECT_EQ(popup.GetLayoutOffsetY(), targetY + target->GetFinalHeight() + 9);

    popup.SetPlacement(NeoWPF::UI::PopupPlacement::Top);
    popup.Arrange(80, 20);
    EXPECT_EQ(popup.GetLayoutOffsetX(), targetX + 5);
    EXPECT_EQ(popup.GetLayoutOffsetY(), targetY - 20 + 9);

    popup.SetPlacement(NeoWPF::UI::PopupPlacement::Left);
    popup.Arrange(80, 20);
    EXPECT_EQ(popup.GetLayoutOffsetX(), targetX - 80 + 5);
    EXPECT_EQ(popup.GetLayoutOffsetY(), targetY + 9);

    popup.SetPlacement(NeoWPF::UI::PopupPlacement::Right);
    popup.Arrange(80, 20);
    EXPECT_EQ(popup.GetLayoutOffsetX(), targetX + target->GetFinalWidth() + 5);
    EXPECT_EQ(popup.GetLayoutOffsetY(), targetY + 9);
}

TEST_F(PopupTests, OverlayOwnershipTracksOpenAndClose)
{
    auto window = std::make_shared<NeoWPF::UI::Window>();
    auto root = std::make_shared<NeoWPF::UI::StackPanel>();
    auto target = std::make_shared<NeoWPF::UI::Button>(L"Target");
    window->SetContent(root);
    root->AddChild(target);

    auto popup = std::make_shared<NeoWPF::UI::Popup>();
    popup->SetPlacementTarget(target.get());
    popup->SetIsOpen(true);

    ASSERT_EQ(window->GetOverlays().size(), 1u);
    EXPECT_EQ(window->GetOverlays().front().get(), static_cast<NeoWPF::UI::Visual*>(popup.get()));

    popup->SetIsOpen(true);
    EXPECT_EQ(window->GetOverlays().size(), 1u);

    popup->SetIsOpen(false);
    EXPECT_TRUE(window->GetOverlays().empty());
}

TEST_F(PopupTests, OverlayHitTestingChecksTopmostOpenPopupFirst)
{
    auto lower = std::make_shared<NeoWPF::UI::Popup>();
    auto upper = std::make_shared<NeoWPF::UI::Popup>();
    auto lowerButton = std::make_shared<NeoWPF::UI::Button>(L"Lower");
    auto upperButton = std::make_shared<NeoWPF::UI::Button>(L"Upper");

    lower->SetContent(lowerButton);
    upper->SetContent(upperButton);
    lower->SetIsOpen(true);
    upper->SetIsOpen(true);
    lower->Measure(200, 60);
    upper->Measure(200, 60);
    lower->Arrange(200, 60);
    upper->Arrange(200, 60);

    auto& input = NeoWPF::Input::InputManager::GetInstance();
    input.SetRootElement(lower.get());
    input.SetOverlayElements({lower, upper});

    EXPECT_EQ(input.HitTest(10, 10), upperButton.get());

    upper->SetIsOpen(false);
    input.SetOverlayElements({lower, upper});
    EXPECT_EQ(input.HitTest(10, 10), lowerButton.get());
}

TEST_F(PopupTests, SharedPlacementConverterRejectsMalformedValues)
{
    const auto converter = NeoWPF::UI::Properties::GetPopupPlacementConverter();
    ASSERT_NE(converter, nullptr);

    std::any converted;
    EXPECT_TRUE(converter->ConvertFromString("Top", converted));
    ASSERT_EQ(converted.type(), typeid(NeoWPF::UI::PopupPlacement));
    EXPECT_EQ(std::any_cast<NeoWPF::UI::PopupPlacement>(converted), NeoWPF::UI::PopupPlacement::Top);

    converted.reset();
    EXPECT_FALSE(converter->ConvertFromString("diagonal", converted));
    EXPECT_FALSE(converted.has_value());

    std::string expression;
    EXPECT_TRUE(converter->TryConvertToCode("bottom", expression));
    EXPECT_EQ(expression, "NeoWPF::UI::PopupPlacement::Bottom");
    EXPECT_FALSE(converter->TryConvertToCode("", expression));
    EXPECT_TRUE(expression.empty());
}

TEST_F(PopupTests, MalformedDependencyPropertyValuesDoNotChangePopupState)
{
    NeoWPF::UI::Popup popup;
    popup.SetPlacement(NeoWPF::UI::PopupPlacement::Left);
    popup.SetHorizontalOffset(21);

    const auto& placement = NeoWPF::UI::Popup::PlacementProperty();
    const auto& offset = NeoWPF::UI::Popup::HorizontalOffsetProperty();

    EXPECT_NO_THROW(popup.SetValue(placement, std::string("not-a-placement")));
    EXPECT_NO_THROW(popup.SetValue(offset, std::string("not-an-integer")));
    EXPECT_EQ(popup.GetPlacement(), NeoWPF::UI::PopupPlacement::Left);
    EXPECT_EQ(popup.GetHorizontalOffset(), 21);
}

TEST_F(PopupTests, NullPlacementTargetIsSafe)
{
    NeoWPF::UI::Popup popup;

    EXPECT_NO_THROW(popup.SetPlacementTarget(nullptr));
    EXPECT_NO_THROW(popup.SetIsOpen(true));
    EXPECT_NO_THROW(popup.Arrange(100, 40));
    EXPECT_EQ(popup.GetPlacementTarget(), nullptr);
}

}
