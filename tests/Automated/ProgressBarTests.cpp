#include <gtest/gtest.h>

#include <NeoWPF/UI/Controls/ProgressBar/ProgressBar.h>

namespace {

TEST(ProgressBarTests, UsesExpectedDefaults)
{
    NeoWPF::UI::ProgressBar progressBar;

    EXPECT_DOUBLE_EQ(progressBar.GetMinimum(), 0.0);
    EXPECT_DOUBLE_EQ(progressBar.GetMaximum(), 100.0);
    EXPECT_DOUBLE_EQ(progressBar.GetValue(), 0.0);
    EXPECT_EQ(progressBar.GetOrientation(), NeoWPF::UI::Orientation::Horizontal);
    EXPECT_FALSE(progressBar.GetIsIndeterminate());
}

TEST(ProgressBarTests, ClampsProgressValues)
{
    NeoWPF::UI::ProgressBar progressBar;

    progressBar.SetValue(-10.0);
    EXPECT_DOUBLE_EQ(progressBar.GetValue(), 0.0);

    progressBar.SetValue(150.0);
    EXPECT_DOUBLE_EQ(progressBar.GetValue(), 100.0);
}

TEST(ProgressBarTests, ConfiguresOrientationAndIndeterminateMode)
{
    NeoWPF::UI::ProgressBar progressBar;

    progressBar.SetOrientation(NeoWPF::UI::Orientation::Vertical);
    progressBar.SetIsIndeterminate(true);

    EXPECT_EQ(progressBar.GetOrientation(), NeoWPF::UI::Orientation::Vertical);
    EXPECT_TRUE(progressBar.GetIsIndeterminate());

    progressBar.SetIsIndeterminate(false);
    EXPECT_FALSE(progressBar.GetIsIndeterminate());
}

TEST(ProgressBarTests, HandlesDegenerateRange)
{
    NeoWPF::UI::ProgressBar progressBar;

    progressBar.SetMinimum(50.0);
    progressBar.SetMaximum(50.0);
    progressBar.SetValue(100.0);

    EXPECT_DOUBLE_EQ(progressBar.GetMinimum(), 50.0);
    EXPECT_DOUBLE_EQ(progressBar.GetMaximum(), 50.0);
    EXPECT_DOUBLE_EQ(progressBar.GetValue(), 50.0);
}

TEST(ProgressBarTests, RaisesValueChangedForProgressUpdates)
{
    NeoWPF::UI::ProgressBar progressBar;
    int valueChangedCount = 0;

    progressBar.ValueChanged += [&valueChangedCount](double) {
        ++valueChangedCount;
    };

    progressBar.SetValue(42.0);

    EXPECT_EQ(valueChangedCount, 1);
}

}
