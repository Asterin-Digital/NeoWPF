#include <gtest/gtest.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Keyboard.h>

#if defined(_WIN32)
#include <NeoWPF/Platform/Windows/WWindowing.h>
#include <windows.h>
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