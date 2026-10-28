#include <gtest/gtest.h>

#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/Input/Keyboard.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>

namespace {

class TestTextBox final : public NeoWPF::UI::TextBox {
public:
    using NeoWPF::UI::TextBox::BuildRenderCommands;
};

void RaisePointerEvent(NeoWPF::UI::TextBox& textBox,
                       const NeoWPF::Input::Core::RoutedEvent& routedEvent,
                       NeoWPF::Input::Core::PointerPhase phase,
                       double x,
                       double y,
                       double wheelDelta = 0.0)
{
    const NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse, phase,
        x, y, 0x01u, 0x01u, 0.0f, wheelDelta);
    NeoWPF::Input::Core::RoutedPointerEventArgs args(routedEvent, packet);
    args.SetSource(&textBox);
    args.SetOriginalSource(&textBox);
    textBox.RaiseEvent(&args);
}

void RaiseKeyDown(NeoWPF::UI::TextBox& textBox,
                  NeoWPF::Input::Key key,
                  wchar_t character = 0)
{
    NeoWPF::Input::Core::RoutedKeyboardEventArgs args(
        NeoWPF::Input::Core::KeyDownEvent(), static_cast<int>(key), true, character);
    args.SetSource(&textBox);
    args.SetOriginalSource(&textBox);
    textBox.RaiseEvent(&args);
}

TEST(TextBoxTests, PointerPressPlacesCaretBeforeSubsequentCharacterInput)
{
    TestTextBox textBox;
    textBox.SetWidth(200.0);
    textBox.SetHeight(40.0);
    textBox.SetText(L"abc");
    textBox.Measure(200.0, 40.0);
    textBox.Arrange(200.0, 40.0);

    RaiseKeyDown(textBox, NeoWPF::Input::Key::End);
    RaisePointerEvent(textBox, NeoWPF::Input::Core::PointerPressedEvent(),
                      NeoWPF::Input::Core::PointerPhase::Down, 0.0, 10.0);
    RaiseKeyDown(textBox, NeoWPF::Input::Key::X, L'x');

    EXPECT_EQ(textBox.GetText(), L"xabc");
}

TEST(TextBoxTests, PointerWheelScrollInvalidatesVisualAndMarksEventHandled)
{
    TestTextBox textBox;
    textBox.SetWidth(120.0);
    textBox.SetHeight(10.0);
    textBox.SetText(L"One\nTwo\nThree\nFour\nFive\nSix\nSeven\nEight\nNine\nTen");
    textBox.Measure(120.0, 10.0);
    textBox.Arrange(120.0, 10.0);
    textBox.ClearDirty();

    const NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse,
        NeoWPF::Input::Core::PointerPhase::Wheel,
        10.0, 10.0, 0, 0, 0.0f, -120.0);
    NeoWPF::Input::Core::RoutedPointerEventArgs args(
        NeoWPF::Input::Core::PointerWheelEvent(), packet);
    args.SetSource(&textBox);
    args.SetOriginalSource(&textBox);
    textBox.RaiseEvent(&args);

    EXPECT_TRUE(args.IsHandled());
    EXPECT_TRUE(textBox.IsDirty());
}

} // namespace