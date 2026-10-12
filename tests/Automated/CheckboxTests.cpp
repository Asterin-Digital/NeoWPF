#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>

#include <cmath>
#include <memory>
#include <utility>
#include <vector>

namespace {

class CheckboxTests : public ::testing::Test {
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

using RoundedRectData = decltype(std::declval<NeoWPF::Rendering::CommandList::Command>().roundedRect);

struct Point {
    double x;
    double y;
};

Point RotatePoint(Point point, Point pivot, float angle)
{
    const double cosine = std::cos(angle);
    const double sine = std::sin(angle);
    const double relativeX = point.x - pivot.x;
    const double relativeY = point.y - pivot.y;
    return {
        pivot.x + relativeX * cosine - relativeY * sine,
        pivot.y + relativeX * sine + relativeY * cosine};
}

Point GetRenderedCenter(const RoundedRectData& rectangle)
{
    return RotatePoint(
        {rectangle.x + rectangle.w * 0.5, rectangle.y + rectangle.h * 0.5},
        {rectangle.pivotX, rectangle.pivotY},
        rectangle.rotationRadians);
}

std::vector<const RoundedRectData*> FindRoundedRects(
    const NeoWPF::Rendering::CommandList& commands,
    std::uint32_t color)
{
    std::vector<const RoundedRectData*> matches;
    for (const auto& command : commands.commands) {
        if (command.kind == NeoWPF::Rendering::CommandList::CommandKind::RoundedRect &&
            command.roundedRect.color == color) {
            matches.push_back(&command.roundedRect);
        }
    }
    return matches;
}

std::unique_ptr<NeoWPF::UI::Checkbox> MakeCheckedCheckbox(std::uint32_t checkColor)
{
    auto checkbox = std::make_unique<NeoWPF::UI::Checkbox>();
    checkbox->SetForeground(std::make_shared<NeoWPF::UI::SolidColorBrush>(checkColor));
    checkbox->ApplyTemplate();
    checkbox->Initialize();
    checkbox->Arrange(120.0, 40.0);
    checkbox->SetChecked(true);
    return checkbox;
}

TEST_F(CheckboxTests, CheckmarkShapeRotationKeepsStrokeCentersFixed)
{
    constexpr std::uint32_t checkColor = 0xFF010203u;
    constexpr double firstCenterX = 6.5;
    constexpr double firstCenterY = 21.0;
    constexpr double secondCenterX = 10.8;
    constexpr double secondCenterY = 19.0;

    auto checkbox = MakeCheckedCheckbox(checkColor);

    NeoWPF::Rendering::CommandList commands;
    checkbox->BuildRenderCommands(commands);
    const auto strokes = FindRoundedRects(commands, checkColor);

    ASSERT_EQ(strokes.size(), 2u);
    const auto firstCenter = GetRenderedCenter(*strokes[0]);
    const auto secondCenter = GetRenderedCenter(*strokes[1]);
    EXPECT_NEAR(firstCenter.x, firstCenterX, 0.0001);
    EXPECT_NEAR(firstCenter.y, firstCenterY, 0.0001);
    EXPECT_NEAR(secondCenter.x, secondCenterX, 0.0001);
    EXPECT_NEAR(secondCenter.y, secondCenterY, 0.0001);
}

TEST_F(CheckboxTests, CheckmarkAndBoxFollowCheckboxRotation)
{
    constexpr std::uint32_t checkColor = 0xFF010203u;
    constexpr float controlAngle = 0.35f;
    const Point controlPivot{60.0, 20.0};
    const Point originalFirstStrokeCenter{6.5, 21.0};
    const Point originalSecondStrokeCenter{10.8, 19.0};
    const Point originalBoxCenter{9.0, 20.0};

    auto checkbox = MakeCheckedCheckbox(checkColor);
    checkbox->SetRenderTransformOrigin({0.5f, 0.5f});
    checkbox->SetRenderTransform(std::make_shared<NeoWPF::UI::RotateTransform>(controlAngle));

    NeoWPF::Rendering::CommandList commands;
    checkbox->BuildRenderCommands(commands);
    const auto strokes = FindRoundedRects(commands, checkColor);
    ASSERT_EQ(strokes.size(), 2u);

    const auto firstCenter = GetRenderedCenter(*strokes[0]);
    const auto secondCenter = GetRenderedCenter(*strokes[1]);
    const auto expectedFirstCenter = RotatePoint(originalFirstStrokeCenter, controlPivot, controlAngle);
    const auto expectedSecondCenter = RotatePoint(originalSecondStrokeCenter, controlPivot, controlAngle);
    EXPECT_NEAR(firstCenter.x, expectedFirstCenter.x, 0.0001);
    EXPECT_NEAR(firstCenter.y, expectedFirstCenter.y, 0.0001);
    EXPECT_NEAR(secondCenter.x, expectedSecondCenter.x, 0.0001);
    EXPECT_NEAR(secondCenter.y, expectedSecondCenter.y, 0.0001);

    std::size_t boxCommandCount = 0;
    const auto expectedBoxCenter = RotatePoint(originalBoxCenter, controlPivot, controlAngle);
    for (const auto& command : commands.commands) {
        if (command.kind != NeoWPF::Rendering::CommandList::CommandKind::RoundedRect ||
            command.roundedRect.color == checkColor) {
            continue;
        }

        ++boxCommandCount;
        const auto boxCenter = GetRenderedCenter(command.roundedRect);
        EXPECT_NEAR(boxCenter.x, expectedBoxCenter.x, 0.0001);
        EXPECT_NEAR(boxCenter.y, expectedBoxCenter.y, 0.0001);
    }
    EXPECT_EQ(boxCommandCount, 2u);
}

}
