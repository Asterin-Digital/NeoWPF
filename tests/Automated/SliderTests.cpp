#include <gtest/gtest.h>

#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Controls/Slider/Slider.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>

#include <memory>

namespace {

void RaisePointerPress(NeoWPF::UI::Slider& slider,
                       NeoWPF::UI::UIElement* originalSource,
                       double x,
                       double y)
{
    const NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse,
        NeoWPF::Input::Core::PointerPhase::Down,
        x, y, 0x01u, 0x01u);
    NeoWPF::Input::Core::RoutedPointerEventArgs args(
        NeoWPF::Input::Core::PointerPressedEvent(), packet);
    args.SetSource(originalSource);
    args.SetOriginalSource(originalSource);
    slider.RaiseEvent(&args);
}

TEST(SliderTests, UpdatesRangeAndRaisesValueChanged)
{
    NeoWPF::UI::Slider slider;
    slider.SetMinimum(10.0);
    slider.SetMaximum(90.0);
    slider.SetSmallChange(2.0);
    slider.SetLargeChange(20.0);

    int valueChangedCount = 0;
    double lastValue = -1.0;

    slider.ValueChanged += [&valueChangedCount, &lastValue](double value) {
        ++valueChangedCount;
        lastValue = value;
    };

    slider.SetValue(42.5);

    EXPECT_DOUBLE_EQ(slider.GetMinimum(), 10.0);
    EXPECT_DOUBLE_EQ(slider.GetMaximum(), 90.0);
    EXPECT_DOUBLE_EQ(slider.GetSmallChange(), 2.0);
    EXPECT_DOUBLE_EQ(slider.GetLargeChange(), 20.0);
    EXPECT_DOUBLE_EQ(slider.GetValue(), 42.5);
    EXPECT_EQ(valueChangedCount, 1);
    EXPECT_DOUBLE_EQ(lastValue, 42.5);
}

TEST(SliderTests, ConfiguresOrientationDirectionAndSelection)
{
    NeoWPF::UI::Slider slider;

    slider.SetOrientation(NeoWPF::UI::Orientation::Vertical);
    slider.SetIsDirectionReversed(true);
    slider.SetIsSelectionRangeEnabled(true);
    slider.SetSelectionStart(20.0);
    slider.SetSelectionEnd(80.0);

    EXPECT_EQ(slider.GetOrientation(), NeoWPF::UI::Orientation::Vertical);
    EXPECT_TRUE(slider.GetIsDirectionReversed());
    EXPECT_TRUE(slider.GetIsSelectionRangeEnabled());
    EXPECT_DOUBLE_EQ(slider.GetSelectionStart(), 20.0);
    EXPECT_DOUBLE_EQ(slider.GetSelectionEnd(), 80.0);
}

TEST(SliderTests, SnapsToExplicitTicks)
{
    NeoWPF::UI::Slider slider;
    slider.SetMinimum(0.0);
    slider.SetMaximum(100.0);
    slider.SetTickFrequency(0.0);
    slider.SetTicks({0.0, 25.0, 50.0, 75.0, 100.0});
    slider.SetIsSnapToTickEnabled(true);

    slider.SetValue(63.0);

    EXPECT_DOUBLE_EQ(slider.GetValue(), 75.0);
}

TEST(SliderTests, ClampsRangeValuesAndChangeAmounts)
{
    NeoWPF::UI::Slider slider;
    slider.SetMinimum(10.0);
    slider.SetMaximum(90.0);

    slider.SetValue(-1000.0);
    EXPECT_DOUBLE_EQ(slider.GetValue(), 10.0);

    slider.SetValue(1000.0);
    EXPECT_DOUBLE_EQ(slider.GetValue(), 90.0);

    slider.SetSmallChange(-1.0);
    slider.SetLargeChange(-1.0);
    EXPECT_DOUBLE_EQ(slider.GetSmallChange(), 0.0);
    EXPECT_DOUBLE_EQ(slider.GetLargeChange(), 0.0);
}

TEST(SliderTests, ClampsSelectionAndTickFrequency)
{
    NeoWPF::UI::Slider slider;
    slider.SetMinimum(20.0);
    slider.SetMaximum(80.0);
    slider.SetSelectionStart(-100.0);
    slider.SetSelectionEnd(100.0);
    slider.SetTickFrequency(-5.0);

    EXPECT_DOUBLE_EQ(slider.GetSelectionStart(), 20.0);
    EXPECT_DOUBLE_EQ(slider.GetSelectionEnd(), 80.0);
    EXPECT_DOUBLE_EQ(slider.GetTickFrequency(), 0.0);
}

TEST(SliderTests, HandlesDegenerateRange)
{
    NeoWPF::UI::Slider slider;
    slider.SetMinimum(50.0);
    slider.SetMaximum(50.0);
    slider.SetValue(1000.0);

    EXPECT_DOUBLE_EQ(slider.GetMinimum(), 50.0);
    EXPECT_DOUBLE_EQ(slider.GetMaximum(), 50.0);
    EXPECT_DOUBLE_EQ(slider.GetValue(), 50.0);
}

TEST(SliderTests, PressingThumbDoesNotJumpValueButTrackPressDoes)
{
    NeoWPF::UI::Slider slider;
    slider.SetWidth(200.0);
    slider.SetHeight(24.0);
    slider.SetValue(50.0);
    slider.ApplyTemplate();
    slider.Initialize();
    slider.Measure(200.0, 24.0);
    slider.Arrange(200.0, 24.0);

    const auto thumb = std::dynamic_pointer_cast<NeoWPF::UI::Thumb>(
        slider.FindTemplateChild("PART_Thumb"));
    ASSERT_NE(thumb, nullptr);

    RaisePointerPress(slider, thumb.get(), 180.0, 12.0);
    EXPECT_DOUBLE_EQ(slider.GetValue(), 50.0);

    RaisePointerPress(slider, &slider, 180.0, 12.0);
    EXPECT_GT(slider.GetValue(), 50.0);
}

}
