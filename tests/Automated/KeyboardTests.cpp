#include <gtest/gtest.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Keyboard.h>

#if defined(_WIN32)
#include <NeoWPF/Platform/Windows/WWindowing.h>
#include <windows.h>
#elif defined(__ANDROID__)
#include <NeoWPF/Platform/Android/AWindowing.h>
#include <android/keycodes.h>
#elif defined(__linux__)
#include <NeoWPF/Platform/Linux/LWindow.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#endif

using NeoWPF::Input::InputManager;
using NeoWPF::Input::Key;
using NeoWPF::Input::Keyboard;

TEST(KeyboardTests, KeyValuesUseTheirAssignedRanges)
{
    EXPECT_EQ(static_cast<int>(Key::None), 0);
    EXPECT_GE(static_cast<int>(Key::Back), 1);
    EXPECT_LE(static_cast<int>(Key::Escape), 9);
    EXPECT_GE(static_cast<int>(Key::Shift), 10);
    EXPECT_LE(static_cast<int>(Key::Alt), 19);
    EXPECT_GE(static_cast<int>(Key::PageUp), 20);
    EXPECT_LE(static_cast<int>(Key::Down), 29);
    EXPECT_GE(static_cast<int>(Key::Insert), 30);
    EXPECT_LE(static_cast<int>(Key::Delete), 39);
    EXPECT_GE(static_cast<int>(Key::D0), 40);
    EXPECT_LE(static_cast<int>(Key::D9), 59);
    EXPECT_GE(static_cast<int>(Key::A), 60);
    EXPECT_LE(static_cast<int>(Key::Z), 89);
    EXPECT_GE(static_cast<int>(Key::F1), 90);
    EXPECT_LE(static_cast<int>(Key::F24), 119);
    EXPECT_GE(static_cast<int>(Key::NumPad0), 120);
    EXPECT_LE(static_cast<int>(Key::Divide), 149);
    EXPECT_GE(static_cast<int>(Key::Space), 150);
    EXPECT_LE(static_cast<int>(Key::Oem102), 179);
    EXPECT_GE(static_cast<int>(Key::VolumeMute), 180);
    EXPECT_LE(static_cast<int>(Key::MediaPlayPause), 199);
}

TEST(KeyboardTests, CharacterQueriesUseRemappedKeyValues)
{
    auto& input = InputManager::GetInstance();
    input.ProcessKeyboardInput(static_cast<int>(Key::A), true, L'a');
    EXPECT_TRUE(Keyboard::IsKeyDown('a'));
    input.ProcessKeyboardInput(static_cast<int>(Key::A), false, L'a');
    EXPECT_FALSE(Keyboard::IsKeyDown('a'));

    input.ProcessKeyboardInput(static_cast<int>(Key::Oem1), true, L';');
    EXPECT_TRUE(Keyboard::IsKeyDown(';'));
    input.ProcessKeyboardInput(static_cast<int>(Key::Oem1), false, L';');
    EXPECT_FALSE(Keyboard::IsKeyDown(';'));
}

TEST(KeyboardTests, NoneAndInvalidCodesDoNotChangeKeyState)
{
    auto& input = InputManager::GetInstance();
    input.ProcessKeyboardInput(static_cast<int>(Key::None), true);
    EXPECT_FALSE(Keyboard::IsKeyDown(Key::None));

    input.ProcessKeyboardInput(-1, true, L'\u00E9');
    EXPECT_FALSE(Keyboard::IsKeyDown(Key::None));
}

#if defined(_WIN32)
TEST(KeyboardTests, WindowsVirtualKeysMapToNormalizedCodes)
{
    using NeoWPF::Platform::Windows::TranslateVirtualKey;

    EXPECT_EQ(TranslateVirtualKey(VK_LMENU), static_cast<int>(Key::Alt));
    EXPECT_EQ(TranslateVirtualKey(VK_F13), static_cast<int>(Key::F13));
    EXPECT_EQ(TranslateVirtualKey(VK_NUMPAD7), static_cast<int>(Key::NumPad7));
    EXPECT_EQ(TranslateVirtualKey(VK_OEM_1), static_cast<int>(Key::Oem1));
    EXPECT_EQ(TranslateVirtualKey(VK_MEDIA_NEXT_TRACK), static_cast<int>(Key::MediaNextTrack));
    EXPECT_EQ(TranslateVirtualKey(VK_SNAPSHOT), static_cast<int>(Key::None));
}
#elif defined(__ANDROID__)
TEST(KeyboardTests, AndroidKeycodesMapToNormalizedCodes)
{
    using NeoWPF::Platform::Android::TranslateAndroidKeyCode;

    for (int keyCode = AKEYCODE_A; keyCode <= AKEYCODE_Z; ++keyCode) {
        EXPECT_EQ(TranslateAndroidKeyCode(keyCode),
                  static_cast<int>(Key::A) + keyCode - AKEYCODE_A);
    }
    EXPECT_EQ(TranslateAndroidKeyCode(AKEYCODE_0), static_cast<int>(Key::D0));
    for (int keyCode = AKEYCODE_1; keyCode <= AKEYCODE_9; ++keyCode) {
        EXPECT_EQ(TranslateAndroidKeyCode(keyCode),
                  static_cast<int>(Key::D1) + keyCode - AKEYCODE_1);
    }
    for (int keyCode = AKEYCODE_F1; keyCode <= AKEYCODE_F12; ++keyCode) {
        EXPECT_EQ(TranslateAndroidKeyCode(keyCode),
                  static_cast<int>(Key::F1) + keyCode - AKEYCODE_F1);
    }
    for (int keyCode = AKEYCODE_NUMPAD_0; keyCode <= AKEYCODE_NUMPAD_9; ++keyCode) {
        EXPECT_EQ(TranslateAndroidKeyCode(keyCode),
                  static_cast<int>(Key::NumPad0) + keyCode - AKEYCODE_NUMPAD_0);
    }

    struct Mapping {
        int androidKeyCode;
        Key key;
    };
    constexpr Mapping mappings[] = {
        {AKEYCODE_DEL, Key::Back},
        {AKEYCODE_TAB, Key::Tab},
        {AKEYCODE_ENTER, Key::Enter},
        {AKEYCODE_NUMPAD_ENTER, Key::Enter},
        {AKEYCODE_BREAK, Key::Pause},
        {AKEYCODE_CAPS_LOCK, Key::CapsLock},
        {AKEYCODE_ESCAPE, Key::Escape},
        {AKEYCODE_SHIFT_LEFT, Key::Shift},
        {AKEYCODE_SHIFT_RIGHT, Key::Shift},
        {AKEYCODE_CTRL_LEFT, Key::Control},
        {AKEYCODE_CTRL_RIGHT, Key::Control},
        {AKEYCODE_ALT_LEFT, Key::Alt},
        {AKEYCODE_ALT_RIGHT, Key::Alt},
        {AKEYCODE_PAGE_UP, Key::PageUp},
        {AKEYCODE_PAGE_DOWN, Key::PageDown},
        {AKEYCODE_MOVE_END, Key::End},
        {AKEYCODE_MOVE_HOME, Key::Home},
        {AKEYCODE_DPAD_LEFT, Key::Left},
        {AKEYCODE_DPAD_UP, Key::Up},
        {AKEYCODE_DPAD_RIGHT, Key::Right},
        {AKEYCODE_DPAD_DOWN, Key::Down},
        {AKEYCODE_INSERT, Key::Insert},
        {AKEYCODE_FORWARD_DEL, Key::Delete},
        {AKEYCODE_NUMPAD_MULTIPLY, Key::Multiply},
        {AKEYCODE_NUMPAD_ADD, Key::Add},
        {AKEYCODE_NUMPAD_COMMA, Key::Separator},
        {AKEYCODE_NUMPAD_SUBTRACT, Key::Subtract},
        {AKEYCODE_NUMPAD_DOT, Key::Decimal},
        {AKEYCODE_NUMPAD_DIVIDE, Key::Divide},
        {AKEYCODE_SPACE, Key::Space},
        {AKEYCODE_SEMICOLON, Key::Oem1},
        {AKEYCODE_EQUALS, Key::OemPlus},
        {AKEYCODE_PLUS, Key::OemPlus},
        {AKEYCODE_COMMA, Key::OemComma},
        {AKEYCODE_MINUS, Key::OemMinus},
        {AKEYCODE_PERIOD, Key::OemPeriod},
        {AKEYCODE_SLASH, Key::Oem2},
        {AKEYCODE_GRAVE, Key::Oem3},
        {AKEYCODE_LEFT_BRACKET, Key::Oem4},
        {AKEYCODE_BACKSLASH, Key::Oem5},
        {AKEYCODE_RIGHT_BRACKET, Key::Oem6},
        {AKEYCODE_APOSTROPHE, Key::Oem7},
        {AKEYCODE_RO, Key::Oem102},
        {AKEYCODE_VOLUME_MUTE, Key::VolumeMute},
        {AKEYCODE_VOLUME_DOWN, Key::VolumeDown},
        {AKEYCODE_VOLUME_UP, Key::VolumeUp},
        {AKEYCODE_MEDIA_NEXT, Key::MediaNextTrack},
        {AKEYCODE_MEDIA_PREVIOUS, Key::MediaPreviousTrack},
        {AKEYCODE_MEDIA_STOP, Key::MediaStop},
        {AKEYCODE_MEDIA_PLAY_PAUSE, Key::MediaPlayPause},
    };
    for (const auto& mapping : mappings) {
        EXPECT_EQ(TranslateAndroidKeyCode(mapping.androidKeyCode),
                  static_cast<int>(mapping.key));
    }
    EXPECT_EQ(TranslateAndroidKeyCode(AKEYCODE_BACK), static_cast<int>(Key::None));
    EXPECT_EQ(TranslateAndroidKeyCode(AKEYCODE_PROG_RED), static_cast<int>(Key::None));
    EXPECT_EQ(TranslateAndroidKeyCode(-1), static_cast<int>(Key::None));
}
#elif defined(__linux__)
TEST(KeyboardTests, XkbKeysymsMapToNormalizedCodes)
{
    using NeoWPF::Platform::Linux::TranslateXkbKeysymToVirtualKey;

    EXPECT_EQ(TranslateXkbKeysymToVirtualKey(XKB_KEY_Alt_L), static_cast<std::uint32_t>(Key::Alt));
    EXPECT_EQ(TranslateXkbKeysymToVirtualKey(XKB_KEY_F13), static_cast<std::uint32_t>(Key::F13));
    EXPECT_EQ(TranslateXkbKeysymToVirtualKey(XKB_KEY_KP_7), static_cast<std::uint32_t>(Key::NumPad7));
    EXPECT_EQ(TranslateXkbKeysymToVirtualKey(XKB_KEY_semicolon), static_cast<std::uint32_t>(Key::Oem1));
    EXPECT_EQ(TranslateXkbKeysymToVirtualKey(XKB_KEY_XF86AudioNext),
              static_cast<std::uint32_t>(Key::MediaNextTrack));
    EXPECT_EQ(TranslateXkbKeysymToVirtualKey(0xFFFFFFFFu), static_cast<std::uint32_t>(Key::None));
}
#endif