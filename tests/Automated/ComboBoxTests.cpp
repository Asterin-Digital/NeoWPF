#include <gtest/gtest.h>

#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Controls/ComboBox/ComboBox.h>

#include <cstdint>

namespace {

void RaisePointerEvent(NeoWPF::UI::UIElement& element,
                       const NeoWPF::Input::Core::RoutedEvent& routedEvent,
                       NeoWPF::Input::Core::PointerPhase phase,
                       std::uint32_t changedButton = 0)
{
    const NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse, phase,
        10.0, 10.0, 0, changedButton);
    NeoWPF::Input::Core::RoutedPointerEventArgs args(routedEvent, packet);
    args.SetSource(&element);
    args.SetOriginalSource(&element);
    element.RaiseEvent(&args);
}

TEST(ComboBoxTests, PointerEnterAndExitUpdateItemHighlight)
{
    NeoWPF::UI::ComboBoxItem item;

    EXPECT_FALSE(item.GetIsHighlighted());
    RaisePointerEvent(item, NeoWPF::Input::Core::PointerEnteredEvent(),
                      NeoWPF::Input::Core::PointerPhase::Enter);
    EXPECT_TRUE(item.GetIsHighlighted());

    RaisePointerEvent(item, NeoWPF::Input::Core::PointerExitedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Leave);
    EXPECT_FALSE(item.GetIsHighlighted());
}

TEST(ComboBoxTests, PointerReleaseTogglesDropDownOpenState)
{
    NeoWPF::UI::ComboBox comboBox;

    EXPECT_FALSE(comboBox.GetIsDropDownOpen());
    RaisePointerEvent(comboBox, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);
    EXPECT_TRUE(comboBox.GetIsDropDownOpen());

    RaisePointerEvent(comboBox, NeoWPF::Input::Core::PointerReleasedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Up, 0x01u);
    EXPECT_FALSE(comboBox.GetIsDropDownOpen());
}

} // namespace