#include <NeoWPF/Platform/Android/AWindowing.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Keyboard.h>

#include <android/keycodes.h>

namespace NeoWPF::Platform::Android {

int TranslateAndroidKeyCode(int keyCode) noexcept
{
    using NeoWPF::Input::Key;

    if (keyCode >= AKEYCODE_A && keyCode <= AKEYCODE_Z) {
        return static_cast<int>(Key::A) + keyCode - AKEYCODE_A;
    }
    if (keyCode == AKEYCODE_0) {
        return static_cast<int>(Key::D0);
    }
    if (keyCode >= AKEYCODE_1 && keyCode <= AKEYCODE_9) {
        return static_cast<int>(Key::D1) + keyCode - AKEYCODE_1;
    }
    if (keyCode >= AKEYCODE_F1 && keyCode <= AKEYCODE_F12) {
        return static_cast<int>(Key::F1) + keyCode - AKEYCODE_F1;
    }
    if (keyCode >= AKEYCODE_NUMPAD_0 && keyCode <= AKEYCODE_NUMPAD_9) {
        return static_cast<int>(Key::NumPad0) + keyCode - AKEYCODE_NUMPAD_0;
    }

    switch (keyCode) {
    case AKEYCODE_DEL: return static_cast<int>(Key::Back);
    case AKEYCODE_TAB: return static_cast<int>(Key::Tab);
    case AKEYCODE_ENTER: return static_cast<int>(Key::Enter);
    case AKEYCODE_NUMPAD_ENTER: return static_cast<int>(Key::Enter);
    case AKEYCODE_BREAK: return static_cast<int>(Key::Pause);
    case AKEYCODE_CAPS_LOCK: return static_cast<int>(Key::CapsLock);
    case AKEYCODE_ESCAPE: return static_cast<int>(Key::Escape);
    case AKEYCODE_SHIFT_LEFT:
    case AKEYCODE_SHIFT_RIGHT: return static_cast<int>(Key::Shift);
    case AKEYCODE_CTRL_LEFT:
    case AKEYCODE_CTRL_RIGHT: return static_cast<int>(Key::Control);
    case AKEYCODE_ALT_LEFT:
    case AKEYCODE_ALT_RIGHT: return static_cast<int>(Key::Alt);
    case AKEYCODE_PAGE_UP: return static_cast<int>(Key::PageUp);
    case AKEYCODE_PAGE_DOWN: return static_cast<int>(Key::PageDown);
    case AKEYCODE_MOVE_END: return static_cast<int>(Key::End);
    case AKEYCODE_MOVE_HOME: return static_cast<int>(Key::Home);
    case AKEYCODE_DPAD_LEFT: return static_cast<int>(Key::Left);
    case AKEYCODE_DPAD_UP: return static_cast<int>(Key::Up);
    case AKEYCODE_DPAD_RIGHT: return static_cast<int>(Key::Right);
    case AKEYCODE_DPAD_DOWN: return static_cast<int>(Key::Down);
    case AKEYCODE_INSERT: return static_cast<int>(Key::Insert);
    case AKEYCODE_FORWARD_DEL: return static_cast<int>(Key::Delete);
    case AKEYCODE_NUMPAD_MULTIPLY: return static_cast<int>(Key::Multiply);
    case AKEYCODE_NUMPAD_ADD: return static_cast<int>(Key::Add);
    case AKEYCODE_NUMPAD_COMMA: return static_cast<int>(Key::Separator);
    case AKEYCODE_NUMPAD_SUBTRACT: return static_cast<int>(Key::Subtract);
    case AKEYCODE_NUMPAD_DOT: return static_cast<int>(Key::Decimal);
    case AKEYCODE_NUMPAD_DIVIDE: return static_cast<int>(Key::Divide);
    case AKEYCODE_SPACE: return static_cast<int>(Key::Space);
    case AKEYCODE_SEMICOLON: return static_cast<int>(Key::Oem1);
    case AKEYCODE_EQUALS:
    case AKEYCODE_PLUS: return static_cast<int>(Key::OemPlus);
    case AKEYCODE_COMMA: return static_cast<int>(Key::OemComma);
    case AKEYCODE_MINUS: return static_cast<int>(Key::OemMinus);
    case AKEYCODE_PERIOD: return static_cast<int>(Key::OemPeriod);
    case AKEYCODE_SLASH: return static_cast<int>(Key::Oem2);
    case AKEYCODE_GRAVE: return static_cast<int>(Key::Oem3);
    case AKEYCODE_LEFT_BRACKET: return static_cast<int>(Key::Oem4);
    case AKEYCODE_BACKSLASH: return static_cast<int>(Key::Oem5);
    case AKEYCODE_RIGHT_BRACKET: return static_cast<int>(Key::Oem6);
    case AKEYCODE_APOSTROPHE: return static_cast<int>(Key::Oem7);
    case AKEYCODE_RO: return static_cast<int>(Key::Oem102);
    case AKEYCODE_VOLUME_MUTE: return static_cast<int>(Key::VolumeMute);
    case AKEYCODE_VOLUME_DOWN: return static_cast<int>(Key::VolumeDown);
    case AKEYCODE_VOLUME_UP: return static_cast<int>(Key::VolumeUp);
    case AKEYCODE_MEDIA_NEXT: return static_cast<int>(Key::MediaNextTrack);
    case AKEYCODE_MEDIA_PREVIOUS: return static_cast<int>(Key::MediaPreviousTrack);
    case AKEYCODE_MEDIA_STOP: return static_cast<int>(Key::MediaStop);
    case AKEYCODE_MEDIA_PLAY_PAUSE: return static_cast<int>(Key::MediaPlayPause);
    default: return static_cast<int>(Key::None);
    }
}

int AWindowing::HandleKeyInputEvent(AInputEvent* inputEvent) noexcept
{
    if (m_windows.empty()) {
        return 0;
    }

    const int androidKeyCode = AKeyEvent_getKeyCode(inputEvent);
    const int keyCode = TranslateAndroidKeyCode(androidKeyCode);
    if (keyCode == static_cast<int>(NeoWPF::Input::Key::None)) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "Android key event not handled: unmapped keyCode=%d action=%d",
            androidKeyCode, AKeyEvent_getAction(inputEvent));
        return 0;
    }

    const int action = AKeyEvent_getAction(inputEvent);
    if (action != AKEY_EVENT_ACTION_DOWN && action != AKEY_EVENT_ACTION_UP) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "Android key event not handled: unsupported action=%d keyCode=%d",
            action, androidKeyCode);
        return 0;
    }

    const bool isKeyDown = action == AKEY_EVENT_ACTION_DOWN;
    const int repeatCount = AKeyEvent_getRepeatCount(inputEvent) + 1;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
        NeoWPF::Diagnostics::Level::Debug,
        "Android key event dispatch: keyCode=%d translatedKey=%d action=%s repeat=%d",
        androidKeyCode, keyCode, isKeyDown ? "down" : "up", repeatCount);
    NeoWPF::Core::SetInputRoot(m_windows.front().get());
    NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(
        keyCode, isKeyDown, 0, repeatCount);
    return 1;
}

}