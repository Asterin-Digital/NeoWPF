#include <NeoWPF/Platform/Linux/LKeyboard.h>

#include <NeoWPF/Input/Keyboard.h>

#include <xkbcommon/xkbcommon-keysyms.h>

namespace NeoWPF::Platform::Linux {

std::uint32_t TranslateXkbKeysymToVirtualKey(std::uint32_t keysym) noexcept
{
    using NeoWPF::Input::Key;

    if (keysym >= XKB_KEY_F1 && keysym <= XKB_KEY_F24) {
        return static_cast<std::uint32_t>(static_cast<int>(Key::F1) +
            static_cast<int>(keysym - XKB_KEY_F1));
    }
    if (keysym >= XKB_KEY_KP_0 && keysym <= XKB_KEY_KP_9) {
        return static_cast<std::uint32_t>(static_cast<int>(Key::NumPad0) +
            static_cast<int>(keysym - XKB_KEY_KP_0));
    }
    if (keysym >= XKB_KEY_0 && keysym <= XKB_KEY_9) {
        return static_cast<std::uint32_t>(static_cast<int>(Key::D0) +
            static_cast<int>(keysym - XKB_KEY_0));
    }
    if (keysym >= XKB_KEY_a && keysym <= XKB_KEY_z) {
        return static_cast<std::uint32_t>(static_cast<int>(Key::A) +
            static_cast<int>(keysym - XKB_KEY_a));
    }
    if (keysym >= XKB_KEY_A && keysym <= XKB_KEY_Z) {
        return static_cast<std::uint32_t>(static_cast<int>(Key::A) +
            static_cast<int>(keysym - XKB_KEY_A));
    }

    switch (keysym) {
    case XKB_KEY_BackSpace: return static_cast<std::uint32_t>(Key::Back);
    case XKB_KEY_Tab: return static_cast<std::uint32_t>(Key::Tab);
    case XKB_KEY_Return: return static_cast<std::uint32_t>(Key::Enter);
    case XKB_KEY_Pause: return static_cast<std::uint32_t>(Key::Pause);
    case XKB_KEY_Caps_Lock: return static_cast<std::uint32_t>(Key::CapsLock);
    case XKB_KEY_Escape: return static_cast<std::uint32_t>(Key::Escape);
    case XKB_KEY_Shift_L: case XKB_KEY_Shift_R: return static_cast<std::uint32_t>(Key::Shift);
    case XKB_KEY_Control_L: case XKB_KEY_Control_R: return static_cast<std::uint32_t>(Key::Control);
    case XKB_KEY_Alt_L: case XKB_KEY_Alt_R: return static_cast<std::uint32_t>(Key::Alt);
    case XKB_KEY_Page_Up: return static_cast<std::uint32_t>(Key::PageUp);
    case XKB_KEY_Page_Down: return static_cast<std::uint32_t>(Key::PageDown);
    case XKB_KEY_End: return static_cast<std::uint32_t>(Key::End);
    case XKB_KEY_Home: return static_cast<std::uint32_t>(Key::Home);
    case XKB_KEY_Left: return static_cast<std::uint32_t>(Key::Left);
    case XKB_KEY_Up: return static_cast<std::uint32_t>(Key::Up);
    case XKB_KEY_Right: return static_cast<std::uint32_t>(Key::Right);
    case XKB_KEY_Down: return static_cast<std::uint32_t>(Key::Down);
    case XKB_KEY_Insert: return static_cast<std::uint32_t>(Key::Insert);
    case XKB_KEY_Delete: return static_cast<std::uint32_t>(Key::Delete);
    case XKB_KEY_KP_Enter: return static_cast<std::uint32_t>(Key::Enter);
    case XKB_KEY_space: return static_cast<std::uint32_t>(Key::Space);
    case XKB_KEY_KP_Multiply: return static_cast<std::uint32_t>(Key::Multiply);
    case XKB_KEY_KP_Add: return static_cast<std::uint32_t>(Key::Add);
    case XKB_KEY_KP_Separator: return static_cast<std::uint32_t>(Key::Separator);
    case XKB_KEY_KP_Subtract: return static_cast<std::uint32_t>(Key::Subtract);
    case XKB_KEY_KP_Decimal: return static_cast<std::uint32_t>(Key::Decimal);
    case XKB_KEY_KP_Divide: return static_cast<std::uint32_t>(Key::Divide);
    case XKB_KEY_semicolon: case XKB_KEY_colon: return static_cast<std::uint32_t>(Key::Oem1);
    case XKB_KEY_equal: case XKB_KEY_plus: return static_cast<std::uint32_t>(Key::OemPlus);
    case XKB_KEY_comma: case XKB_KEY_less: return static_cast<std::uint32_t>(Key::OemComma);
    case XKB_KEY_minus: case XKB_KEY_underscore: return static_cast<std::uint32_t>(Key::OemMinus);
    case XKB_KEY_period: case XKB_KEY_greater: return static_cast<std::uint32_t>(Key::OemPeriod);
    case XKB_KEY_slash: case XKB_KEY_question: return static_cast<std::uint32_t>(Key::Oem2);
    case XKB_KEY_grave: case XKB_KEY_asciitilde: return static_cast<std::uint32_t>(Key::Oem3);
    case XKB_KEY_bracketleft: case XKB_KEY_braceleft: return static_cast<std::uint32_t>(Key::Oem4);
    case XKB_KEY_backslash: case XKB_KEY_bar: return static_cast<std::uint32_t>(Key::Oem5);
    case XKB_KEY_bracketright: case XKB_KEY_braceright: return static_cast<std::uint32_t>(Key::Oem6);
    case XKB_KEY_apostrophe: case XKB_KEY_quotedbl: return static_cast<std::uint32_t>(Key::Oem7);
    case XKB_KEY_XF86AudioMute: return static_cast<std::uint32_t>(Key::VolumeMute);
    case XKB_KEY_XF86AudioLowerVolume: return static_cast<std::uint32_t>(Key::VolumeDown);
    case XKB_KEY_XF86AudioRaiseVolume: return static_cast<std::uint32_t>(Key::VolumeUp);
    case XKB_KEY_XF86AudioNext: return static_cast<std::uint32_t>(Key::MediaNextTrack);
    case XKB_KEY_XF86AudioPrev: return static_cast<std::uint32_t>(Key::MediaPreviousTrack);
    case XKB_KEY_XF86AudioStop: return static_cast<std::uint32_t>(Key::MediaStop);
    case XKB_KEY_XF86AudioPlay: return static_cast<std::uint32_t>(Key::MediaPlayPause);
    default: return static_cast<std::uint32_t>(Key::None);
    }
}

}
