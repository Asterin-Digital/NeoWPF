#include <NeoWPF/Platform/Linux/LWindow.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Keyboard.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace NeoWPF::Platform::Linux {
namespace {

void SubmitPointerInput(NeoWPF::Input::Core::PointerPhase phase,
                        double x, double y, std::uint32_t buttons,
                        std::uint32_t changedButton = 0, double wheelDelta = 0.0)
{
    auto& input = static_cast<NeoWPF::Contracts::IPointerInput&>(
        NeoWPF::Input::InputManager::GetInstance());
    input.SubmitPointerEvent(NeoWPF::Input::Core::PointerEventArgs(
        0,
        NeoWPF::Input::Core::PointerDeviceType::Mouse, phase,
        x, y, buttons, changedButton, 0.0f, wheelDelta));
}

const wl_pointer_listener PointerListener = {
    &LWindow::HandlePointerEnter,
    &LWindow::HandlePointerLeave,
    &LWindow::HandlePointerMotion,
    &LWindow::HandlePointerButton,
    &LWindow::HandlePointerAxis,
};

const wl_keyboard_listener KeyboardListener = {
    &LWindow::HandleKeyboardKeymap,
    &LWindow::HandleKeyboardEnter,
    &LWindow::HandleKeyboardLeave,
    &LWindow::HandleKeyboardKey,
    &LWindow::HandleKeyboardModifiers,
};

}

void LWindow::AttachInputDevices()
{
    if (!m_surface || !m_display || !m_seat) {
        return;
    }

    m_pointer = wl_seat_get_pointer(m_seat);
    if (m_pointer) {
        wl_pointer_add_listener(m_pointer, &PointerListener, this);
    }
    m_keyboard = wl_seat_get_keyboard(m_seat);
    if (m_keyboard) {
        wl_keyboard_add_listener(m_keyboard, &KeyboardListener, this);
    }
}

std::uint32_t LWindow::WaylandButtonToNeoWPFButton(std::uint32_t button) const noexcept
{
    switch (button) {
    case 0x110:
        return 0x01;
    case 0x111:
        return 0x02;
    case 0x112:
        return 0x04;
    default:
        return 0;
    }
}

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

void LWindow::UpdateKeyStateFromWayland(std::uint32_t key, bool isPressed, wchar_t charCode)
{
    if (key == 0 && charCode == 0) {
        return;
    }
    NeoWPF::Core::SetInputRoot(this);
    NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(
        key == 0 ? -1 : static_cast<int>(key), isPressed, charCode, 1);
}

void LWindow::ProcessKeyRepeat()
{
    if (!m_repeatActive || !m_xkbState) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    while (now >= m_repeatNext) {
        const auto keysym = xkb_state_key_get_one_sym(m_xkbState, m_repeatKey + 8u);
        const auto virtualKey = TranslateXkbKeysymToVirtualKey(keysym);
        const auto codePoint = xkb_state_key_get_utf32(m_xkbState, m_repeatKey + 8u);
        if (virtualKey == 0 && codePoint == 0) {
            break;
        }
        UpdateKeyStateFromWayland(virtualKey, true, static_cast<wchar_t>(codePoint));
        m_repeatNext += std::chrono::milliseconds(40);
    }
}

void LWindow::HandlePointerEnter(void* data, wl_pointer*, std::uint32_t, wl_surface*, wl_fixed_t sx, wl_fixed_t sy)
{
    auto* window = static_cast<LWindow*>(data);
    NeoWPF::Core::SetInputRoot(window);
    window->m_pointerX = wl_fixed_to_int(sx);
    window->m_pointerY = wl_fixed_to_int(sy);
    SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Enter,
                       window->m_pointerX, window->m_pointerY,
                       window->m_pointerButtonState);
}

void LWindow::HandlePointerLeave(void* data, wl_pointer*, std::uint32_t, wl_surface*)
{
    auto* window = static_cast<LWindow*>(data);
    if (window) {
        NeoWPF::Core::SetInputRoot(window);
        SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Leave,
                           window->m_pointerX, window->m_pointerY,
                           window->m_pointerButtonState);
    }
}

void LWindow::HandlePointerMotion(void* data, wl_pointer*, std::uint32_t, wl_fixed_t sx, wl_fixed_t sy)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window) {
        return;
    }
    NeoWPF::Core::SetInputRoot(window);
    window->m_pointerX = wl_fixed_to_int(sx);
    window->m_pointerY = wl_fixed_to_int(sy);
    SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Move,
                       window->m_pointerX, window->m_pointerY,
                       window->m_pointerButtonState);
}

void LWindow::HandlePointerButton(void* data, wl_pointer*, std::uint32_t serial, std::uint32_t, std::uint32_t button,
                                 std::uint32_t state)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window) {
        return;
    }
    window->m_windowing.RegisterInputSerial(serial);
    const auto bit = window->WaylandButtonToNeoWPFButton(button);
    if (bit == 0) {
        return;
    }
    NeoWPF::Core::SetInputRoot(window);
    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
        window->m_pointerButtonState |= bit;
    } else {
        window->m_pointerButtonState &= ~bit;
    }
    SubmitPointerInput(
        state == WL_POINTER_BUTTON_STATE_PRESSED
            ? NeoWPF::Input::Core::PointerPhase::Down
            : NeoWPF::Input::Core::PointerPhase::Up,
        window->m_pointerX, window->m_pointerY,
        window->m_pointerButtonState, bit);
}

void LWindow::HandlePointerAxis(void* data, wl_pointer*, std::uint32_t, std::uint32_t axis, wl_fixed_t value)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window) {
        return;
    }
    NeoWPF::Core::SetInputRoot(window);
    const int wheelDelta = (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) ? static_cast<int>(wl_fixed_to_double(value) * 120.0) : 0;
    SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Wheel,
                       window->m_pointerX, window->m_pointerY,
                       window->m_pointerButtonState, 0, wheelDelta);
}

void LWindow::HandleKeyboardKeymap(void* data, wl_keyboard*, std::uint32_t, int fd, std::uint32_t size)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window) {
        return;
    }
    char* keymapData = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (keymapData == MAP_FAILED) {
        close(fd);
        return;
    }
    if (!window->m_xkbContext) {
        window->m_xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    }
    if (window->m_xkbContext) {
        window->m_xkbKeymap = xkb_keymap_new_from_string(window->m_xkbContext,
            keymapData, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
        if (window->m_xkbKeymap) {
            if (window->m_xkbState) {
                xkb_state_unref(window->m_xkbState);
                window->m_xkbState = nullptr;
            }
            window->m_xkbState = xkb_state_new(window->m_xkbKeymap);
        }
    }
    munmap(keymapData, size);
    close(fd);
}

void LWindow::HandleKeyboardEnter(void* data, wl_keyboard*, std::uint32_t, wl_surface*, wl_array*)
{
    auto* window = static_cast<LWindow*>(data);
    NeoWPF::Core::SetInputRoot(window);
}

void LWindow::HandleKeyboardLeave(void* data, wl_keyboard*, std::uint32_t, wl_surface*)
{
    auto* window = static_cast<LWindow*>(data);
    if (window) {
        NeoWPF::Core::SetInputRoot(window);
    }
}

void LWindow::HandleKeyboardKey(void* data, wl_keyboard*, std::uint32_t serial, std::uint32_t, std::uint32_t key,
                               std::uint32_t state)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window || !window->m_xkbState) {
        return;
    }
    window->m_windowing.RegisterInputSerial(serial);

    const auto keysym = xkb_state_key_get_one_sym(window->m_xkbState, key + 8u);
    const auto virtualKey = TranslateXkbKeysymToVirtualKey(keysym);
    const bool isPressed = state == WL_KEYBOARD_KEY_STATE_PRESSED;
    const auto codePoint = isPressed
        ? xkb_state_key_get_utf32(window->m_xkbState, key + 8u)
        : 0;
    if (virtualKey != 0 || codePoint != 0) {
        window->UpdateKeyStateFromWayland(virtualKey, isPressed, static_cast<wchar_t>(codePoint));
    }

    if (!isPressed) {
        if (window->m_repeatKey == key) {
            window->m_repeatActive = false;
        }
    } else if (virtualKey != 0 || codePoint != 0) {
        window->m_repeatKey = key;
        window->m_repeatActive = true;
        window->m_repeatNext = std::chrono::steady_clock::now() + std::chrono::milliseconds(600);
    }
}

void LWindow::HandleKeyboardModifiers(void* data, wl_keyboard*, std::uint32_t, std::uint32_t mods_depressed,
                                       std::uint32_t mods_latched, std::uint32_t mods_locked, std::uint32_t group)
{
    auto* window = static_cast<LWindow*>(data);
    if (window && window->m_xkbState) {
        xkb_state_update_mask(window->m_xkbState,
                              mods_depressed,
                              mods_latched,
                              mods_locked,
                              0,
                              0,
                              group);
        NeoWPF::Core::SetInputRoot(window);
    }
}

}
