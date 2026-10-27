#include <NeoWPF/Platform/Linux/LWindow.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Input/InputManager.h>

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

std::uint32_t LWindow::TranslateXkbKeysymToVirtualKey(std::uint32_t keysym) noexcept
{
    switch (keysym) {
    case XKB_KEY_BackSpace: return 0x08;
    case XKB_KEY_Tab: return 0x09;
    case XKB_KEY_Return: return 0x0D;
    case XKB_KEY_Escape: return 0x1B;
    case XKB_KEY_space: return 0x20;
    case XKB_KEY_Shift_L: case XKB_KEY_Shift_R: return 0x10;
    case XKB_KEY_Control_L: case XKB_KEY_Control_R: return 0x11;
    case XKB_KEY_Alt_L: case XKB_KEY_Alt_R: return 0x12;
    case XKB_KEY_Left: return 0x25;
    case XKB_KEY_Up: return 0x26;
    case XKB_KEY_Right: return 0x27;
    case XKB_KEY_Down: return 0x28;
    case XKB_KEY_Delete: return 0x2E;
    case XKB_KEY_Home: return 0x24;
    case XKB_KEY_End: return 0x23;
    case XKB_KEY_a: case XKB_KEY_A: return 0x41;
    case XKB_KEY_b: case XKB_KEY_B: return 0x42;
    case XKB_KEY_c: case XKB_KEY_C: return 0x43;
    case XKB_KEY_d: case XKB_KEY_D: return 0x44;
    case XKB_KEY_e: case XKB_KEY_E: return 0x45;
    case XKB_KEY_f: case XKB_KEY_F: return 0x46;
    case XKB_KEY_g: case XKB_KEY_G: return 0x47;
    case XKB_KEY_h: case XKB_KEY_H: return 0x48;
    case XKB_KEY_i: case XKB_KEY_I: return 0x49;
    case XKB_KEY_j: case XKB_KEY_J: return 0x4A;
    case XKB_KEY_k: case XKB_KEY_K: return 0x4B;
    case XKB_KEY_l: case XKB_KEY_L: return 0x4C;
    case XKB_KEY_m: case XKB_KEY_M: return 0x4D;
    case XKB_KEY_n: case XKB_KEY_N: return 0x4E;
    case XKB_KEY_o: case XKB_KEY_O: return 0x4F;
    case XKB_KEY_p: case XKB_KEY_P: return 0x50;
    case XKB_KEY_q: case XKB_KEY_Q: return 0x51;
    case XKB_KEY_r: case XKB_KEY_R: return 0x52;
    case XKB_KEY_s: case XKB_KEY_S: return 0x53;
    case XKB_KEY_t: case XKB_KEY_T: return 0x54;
    case XKB_KEY_u: case XKB_KEY_U: return 0x55;
    case XKB_KEY_v: case XKB_KEY_V: return 0x56;
    case XKB_KEY_w: case XKB_KEY_W: return 0x57;
    case XKB_KEY_x: case XKB_KEY_X: return 0x58;
    case XKB_KEY_y: case XKB_KEY_Y: return 0x59;
    case XKB_KEY_z: case XKB_KEY_Z: return 0x5A;
    case XKB_KEY_0: return 0x30;
    case XKB_KEY_1: return 0x31;
    case XKB_KEY_2: return 0x32;
    case XKB_KEY_3: return 0x33;
    case XKB_KEY_4: return 0x34;
    case XKB_KEY_5: return 0x35;
    case XKB_KEY_6: return 0x36;
    case XKB_KEY_7: return 0x37;
    case XKB_KEY_8: return 0x38;
    case XKB_KEY_9: return 0x39;
    default: return 0;
    }
}

void LWindow::UpdateKeyStateFromWayland(std::uint32_t key, bool isPressed, wchar_t charCode)
{
    if (key == 0 && charCode == 0) {
        return;
    }
    NeoWPF::Core::SetInputRootForWindow(this);
    NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(
        static_cast<int>(key), isPressed, charCode, 1);
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
    NeoWPF::Core::SetInputRootForWindow(window);
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
        NeoWPF::Core::SetInputRootForWindow(window);
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
    NeoWPF::Core::SetInputRootForWindow(window);
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
    NeoWPF::Core::SetInputRootForWindow(window);
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
    NeoWPF::Core::SetInputRootForWindow(window);
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
    NeoWPF::Core::SetInputRootForWindow(window);
}

void LWindow::HandleKeyboardLeave(void* data, wl_keyboard*, std::uint32_t, wl_surface*)
{
    auto* window = static_cast<LWindow*>(data);
    if (window) {
        NeoWPF::Core::SetInputRootForWindow(window);
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
        NeoWPF::Core::SetInputRootForWindow(window);
    }
}

}
