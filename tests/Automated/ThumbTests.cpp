#include <gtest/gtest.h>

#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>

namespace {

TEST(ThumbTests, NonPrimaryButtonReleaseKeepsCaptureDuringDrag)
{
    NeoWPF::UI::Thumb thumb;
    thumb.SetBounds(0, 0, 20, 20);

    auto& input = NeoWPF::Input::InputManager::GetInstance();
    input.NotifyVisualTreeReplaced();
    input.SetRootElement(&thumb);

    const auto submit = [&input](NeoWPF::Input::Core::PointerPhase phase,
                                 double x, double y,
                                 std::uint32_t buttons,
                                 std::uint32_t changedButton) {
        const NeoWPF::Input::Core::PointerEventArgs args(
            1, NeoWPF::Input::Core::PointerDeviceType::Mouse, phase,
            x, y, buttons, changedButton);
        input.SubmitPointerEvent(args);
    };

    submit(NeoWPF::Input::Core::PointerPhase::Down, 10.0, 10.0, 0x01u, 0x01u);
    EXPECT_TRUE(thumb.IsDragging());
    EXPECT_EQ(input.GetPointerCapture(1), &thumb);

    submit(NeoWPF::Input::Core::PointerPhase::Down, 10.0, 10.0, 0x03u, 0x02u);
    submit(NeoWPF::Input::Core::PointerPhase::Up, 10.0, 10.0, 0x01u, 0x02u);

    EXPECT_TRUE(thumb.IsDragging());
    EXPECT_EQ(input.GetPointerCapture(1), &thumb);

    submit(NeoWPF::Input::Core::PointerPhase::Up, 10.0, 10.0, 0x00u, 0x01u);

    EXPECT_FALSE(thumb.IsDragging());
    EXPECT_EQ(input.GetPointerCapture(1), nullptr);
    input.NotifyVisualTreeReplaced();
}

} // namespace