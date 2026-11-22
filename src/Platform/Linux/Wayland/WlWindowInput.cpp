#include <NeoWPF/Platform/Linux/Wayland/WlWindow.h>
#include <NeoWPF/Platform/Linux/LKeyboard.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Keyboard.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cmath>

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
    &WlWindow::HandlePointerEnter,
    &WlWindow::HandlePointerLeave,
    &WlWindow::HandlePointerMotion,
    &WlWindow::HandlePointerButton,
    &WlWindow::HandlePointerAxis,
};

const wl_keyboard_listener KeyboardListener = {
    &WlWindow::HandleKeyboardKeymap,
    &WlWindow::HandleKeyboardEnter,
    &WlWindow::HandleKeyboardLeave,
    &WlWindow::HandleKeyboardKey,
    &WlWindow::HandleKeyboardModifiers,
};

}

void WlWindow::AttachInputDevices()
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

std::uint32_t WlWindow::WaylandButtonToNeoWPFButton(std::uint32_t button) const noexcept
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


void WlWindow::UpdateKeyStateFromWayland(std::uint32_t key, bool isPressed, wchar_t charCode)
{
    if (key == 0 && charCode == 0) {
        return;
    }
    NeoWPF::Core::SetInputRoot(this);
    NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(
        key == 0 ? -1 : static_cast<int>(key), isPressed, charCode, 1);
}

void WlWindow::ProcessKeyRepeat()
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

void WlWindow::HandlePointerEnter(void* data, wl_pointer*, std::uint32_t, wl_surface*, wl_fixed_t sx, wl_fixed_t sy)
{
    auto* window = static_cast<WlWindow*>(data);
    NeoWPF::Core::SetInputRoot(window);
    const double scaleFactor = window->ScaleFactor();
    if (!std::isfinite(scaleFactor) || scaleFactor <= 0.0) {
        return;
    }
    window->m_pointerX = wl_fixed_to_double(sx) / scaleFactor;
    window->m_pointerY = wl_fixed_to_double(sy) / scaleFactor;
    SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Enter,
                       window->m_pointerX, window->m_pointerY,
                       window->m_pointerButtonState);
}

void WlWindow::HandlePointerLeave(void* data, wl_pointer*, std::uint32_t, wl_surface*)
{
    auto* window = static_cast<WlWindow*>(data);
    if (window) {
        NeoWPF::Core::SetInputRoot(window);
        SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Leave,
                           window->m_pointerX, window->m_pointerY,
                           window->m_pointerButtonState);
    }
}

void WlWindow::HandlePointerMotion(void* data, wl_pointer*, std::uint32_t, wl_fixed_t sx, wl_fixed_t sy)
{
    auto* window = static_cast<WlWindow*>(data);
    if (!window) {
        return;
    }
    NeoWPF::Core::SetInputRoot(window);
    const double scaleFactor = window->ScaleFactor();
    if (!std::isfinite(scaleFactor) || scaleFactor <= 0.0) {
        return;
    }
    window->m_pointerX = wl_fixed_to_double(sx) / scaleFactor;
    window->m_pointerY = wl_fixed_to_double(sy) / scaleFactor;
    SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Move,
                       window->m_pointerX, window->m_pointerY,
                       window->m_pointerButtonState);
}

void WlWindow::HandlePointerButton(void* data, wl_pointer*, std::uint32_t serial, std::uint32_t, std::uint32_t button,
                                 std::uint32_t state)
{
    auto* window = static_cast<WlWindow*>(data);
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

void WlWindow::HandlePointerAxis(void* data, wl_pointer*, std::uint32_t, std::uint32_t axis, wl_fixed_t value)
{
    auto* window = static_cast<WlWindow*>(data);
    if (!window) {
        return;
    }
    NeoWPF::Core::SetInputRoot(window);
    const int wheelDelta = (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) ? static_cast<int>(wl_fixed_to_double(value) * 120.0) : 0;
    SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Wheel,
                       window->m_pointerX, window->m_pointerY,
                       window->m_pointerButtonState, 0, wheelDelta);
}

void WlWindow::HandleKeyboardKeymap(void* data, wl_keyboard*, std::uint32_t, int fd, std::uint32_t size)
{
    auto* window = static_cast<WlWindow*>(data);
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

void WlWindow::HandleKeyboardEnter(void* data, wl_keyboard*, std::uint32_t, wl_surface*, wl_array*)
{
    auto* window = static_cast<WlWindow*>(data);
    NeoWPF::Core::SetInputRoot(window);
}

void WlWindow::HandleKeyboardLeave(void* data, wl_keyboard*, std::uint32_t, wl_surface*)
{
    auto* window = static_cast<WlWindow*>(data);
    if (window) {
        NeoWPF::Core::SetInputRoot(window);
    }
}

void WlWindow::HandleKeyboardKey(void* data, wl_keyboard*, std::uint32_t serial, std::uint32_t, std::uint32_t key,
                               std::uint32_t state)
{
    auto* window = static_cast<WlWindow*>(data);
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

void WlWindow::HandleKeyboardModifiers(void* data, wl_keyboard*, std::uint32_t, std::uint32_t mods_depressed,
                                       std::uint32_t mods_latched, std::uint32_t mods_locked, std::uint32_t group)
{
    auto* window = static_cast<WlWindow*>(data);
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
