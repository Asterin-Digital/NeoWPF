#include <gtest/gtest.h>

#include <NeoWPF/UI/Controls/ProgressRing/ProgressRing.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

#include <memory>

namespace {

TEST(ProgressRingTests, UsesExpectedDefaults)
{
    NeoWPF::UI::ProgressRing progressRing;

    EXPECT_DOUBLE_EQ(progressRing.GetMinimum(), 0.0);
    EXPECT_DOUBLE_EQ(progressRing.GetMaximum(), 100.0);
    EXPECT_DOUBLE_EQ(progressRing.GetValue(), 0.0);
    EXPECT_DOUBLE_EQ(progressRing.GetRingThickness(), 6.0);
    EXPECT_FALSE(progressRing.GetIsIndeterminate());
}

TEST(ProgressRingTests, MeasuresSquareDefaultWithinAvailableSize)
{
    NeoWPF::UI::ProgressRing progressRing;

    progressRing.Measure(40.0, 64.0);

    EXPECT_DOUBLE_EQ(progressRing.GetDesiredWidth(), 40.0);
    EXPECT_DOUBLE_EQ(progressRing.GetDesiredHeight(), 48.0);
}

TEST(ProgressRingTests, MeasuresExplicitWidthAndHeight)
{
    NeoWPF::UI::ProgressRing progressRing;
    progressRing.SetWidth(72.0);
    progressRing.SetHeight(56.0);

    progressRing.Measure(100.0, 100.0);

    EXPECT_DOUBLE_EQ(progressRing.GetDesiredWidth(), 72.0);
    EXPECT_DOUBLE_EQ(progressRing.GetDesiredHeight(), 56.0);
}

TEST(ProgressRingTests, ClampsRingThicknessAtZero)
{
    NeoWPF::UI::ProgressRing progressRing;

    progressRing.SetRingThickness(-2.0);

    EXPECT_DOUBLE_EQ(progressRing.GetRingThickness(), 0.0);
}

TEST(ProgressRingTests, ClampsProgressValuesAndUpdatesProgressFraction)
{
    NeoWPF::UI::ProgressRing progressRing;
    progressRing.SetValue(50.0);

    EXPECT_DOUBLE_EQ(progressRing.GetValue(), 50.0);
    EXPECT_FLOAT_EQ(progressRing.GetAnimationProgress(), 0.5f);

    progressRing.SetValue(120.0);

    EXPECT_DOUBLE_EQ(progressRing.GetValue(), 100.0);
    EXPECT_FLOAT_EQ(progressRing.GetAnimationProgress(), 1.0f);
}

TEST(ProgressRingTests, RangeChangesRefreshDeterminateProgress)
{
    NeoWPF::UI::ProgressRing progressRing;
    progressRing.SetValue(50.0);
    progressRing.SetMaximum(200.0);

    EXPECT_FLOAT_EQ(progressRing.GetAnimationProgress(), 0.25f);
}

TEST(ProgressRingTests, ConfiguresIndeterminateMode)
{
    NeoWPF::UI::ProgressRing progressRing;

    progressRing.SetIsIndeterminate(true);
    EXPECT_TRUE(progressRing.GetIsIndeterminate());

    progressRing.SetIsIndeterminate(false);
    EXPECT_FALSE(progressRing.GetIsIndeterminate());
}

TEST(ProgressRingTests, WrapsIndeterminatePhaseIntoUnitInterval)
{
    NeoWPF::UI::ProgressRing progressRing;

    progressRing.SetIndeterminatePhase(1.25f);
    EXPECT_FLOAT_EQ(progressRing.GetIndeterminatePhase(), 0.25f);

    progressRing.SetIndeterminatePhase(-0.25f);
    EXPECT_FLOAT_EQ(progressRing.GetIndeterminatePhase(), 0.75f);
}

TEST(ProgressRingTests, HandlesDegenerateRange)
{
    NeoWPF::UI::ProgressRing progressRing;
    progressRing.SetMinimum(50.0);
    progressRing.SetMaximum(50.0);

    EXPECT_DOUBLE_EQ(progressRing.GetValue(), 50.0);
    EXPECT_FLOAT_EQ(progressRing.GetAnimationProgress(), 1.0f);
}

TEST(ProgressRingTests, EmitsTrackArcForArrangedBounds)
{
    NeoWPF::UI::ProgressRing progressRing;
    progressRing.SetBackgroundBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF223344u));
    progressRing.Arrange(48.0, 48.0);

    NeoWPF::Rendering::CommandList commands;
    progressRing.BuildRenderCommands(commands);

    ASSERT_FALSE(commands.commands.empty());
    ASSERT_EQ(commands.commands.front().GetKind(), NeoWPF::Rendering::CommandList::CommandKind::Arc);
    EXPECT_DOUBLE_EQ(commands.commands.front().GetArc().centerX, 24.0);
    EXPECT_DOUBLE_EQ(commands.commands.front().GetArc().centerY, 24.0);
    EXPECT_DOUBLE_EQ(commands.commands.front().GetArc().radius, 21.0);
    EXPECT_DOUBLE_EQ(commands.commands.front().GetArc().thickness, 6.0);
    EXPECT_DOUBLE_EQ(commands.commands.front().GetArc().sweepAngleRadians, 6.28318530717958647692);
}

}