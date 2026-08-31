#include <NeoWPF/Platform/Linux/LWindow.h>
#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <algorithm>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

namespace NeoWPF::Platform::Linux {

namespace {

const xdg_surface_listener SurfaceListener = {
    &LWindow::HandleSurfaceConfigure,
};

const xdg_toplevel_listener ToplevelListener = {
    &LWindow::HandleToplevelConfigure,
    &LWindow::HandleToplevelClose,
};

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

LWindow::LWindow(LWindowing& windowing, const wchar_t* title, int width, int height)
    : m_display(windowing.Display()),
      m_title(title ? title : L""),
      m_width((std::max)(width, 1)),
      m_height((std::max)(height, 1)),
      m_seat(windowing.Seat())
{
    if (windowing.IsConnected() && windowing.Compositor()) {
        m_surface = wl_compositor_create_surface(windowing.Compositor());
        if (m_surface && windowing.XdgWmBase()) {
            m_xdgSurface = xdg_wm_base_get_xdg_surface(windowing.XdgWmBase(), m_surface);
            if (m_xdgSurface) {
                xdg_surface_add_listener(m_xdgSurface, &SurfaceListener, this);
                m_xdgToplevel = xdg_surface_get_toplevel(m_xdgSurface);
            }
            if (m_xdgToplevel) {
                xdg_toplevel_add_listener(m_xdgToplevel, &ToplevelListener, this);
                const auto titleUtf8 = NeoWPF::Platform::WideToUtf8(m_title);
                xdg_toplevel_set_title(m_xdgToplevel, titleUtf8.c_str());
                xdg_toplevel_set_app_id(m_xdgToplevel, "NeoWPF");
                wl_surface_commit(m_surface);
                wl_display_flush(m_display);
            }
        }
    }

    if (windowing.Seat()) {
        AttachInputDevices();
    }
}

LWindow::~LWindow()
{
    Close();
}

bool LWindow::IsValid() const noexcept
{
    return m_surface != nullptr && m_xdgSurface != nullptr && m_xdgToplevel != nullptr;
}

void LWindow::Close() noexcept
{
    if (m_keyboard) {
        wl_keyboard_release(m_keyboard);
        m_keyboard = nullptr;
    }
    if (m_pointer) {
        wl_pointer_release(m_pointer);
        m_pointer = nullptr;
    }
    if (m_xkbState) {
        xkb_state_unref(m_xkbState);
        m_xkbState = nullptr;
    }
    if (m_xkbKeymap) {
        xkb_keymap_unref(m_xkbKeymap);
        m_xkbKeymap = nullptr;
    }
    if (m_xkbContext) {
        xkb_context_unref(m_xkbContext);
        m_xkbContext = nullptr;
    }
    if (m_xdgToplevel) {
        xdg_toplevel_destroy(m_xdgToplevel);
        m_xdgToplevel = nullptr;
    }
    if (m_xdgSurface) {
        xdg_surface_destroy(m_xdgSurface);
        m_xdgSurface = nullptr;
    }
    if (m_surface) {
        wl_surface_destroy(m_surface);
        m_surface = nullptr;
    }
}

wl_surface* LWindow::Surface() const noexcept
{
    return m_surface;
}

wl_display* LWindow::Display() const noexcept
{
    return m_display;
}

const std::wstring& LWindow::Title() const noexcept
{
    return m_title;
}

int LWindow::Width() const noexcept
{
    return m_width;
}

int LWindow::Height() const noexcept
{
    return m_height;
}

void LWindow::SetSize(int width, int height) noexcept
{
    m_width = (std::max)(width, 1);
    m_height = (std::max)(height, 1);
}

bool LWindow::IsConfigured() const noexcept
{
    return m_configured;
}

bool LWindow::IsClosed() const noexcept
{
    return m_closed;
}

void LWindow::AcknowledgeConfigure() noexcept
{
    if (m_xdgSurface && m_configureSerial != 0) {
        xdg_surface_ack_configure(m_xdgSurface, m_configureSerial);
        m_configureSerial = 0;
    }
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

void LWindow::UpdateKeyStateFromWayland(std::uint32_t key, bool isPressed)
{
    if (key == 0) {
        return;
    }
    NeoWPF::Core::SetInputRootForWindow(this);
    NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(
        static_cast<int>(key), isPressed, 0, 1);
}

void LWindow::HandlePointerEnter(void* data, wl_pointer*, std::uint32_t, wl_surface*, wl_fixed_t sx, wl_fixed_t sy)
{
    auto* window = static_cast<LWindow*>(data);
    NeoWPF::Core::SetInputRootForWindow(window);
    window->m_pointerX = wl_fixed_to_int(sx);
    window->m_pointerY = wl_fixed_to_int(sy);
}

void LWindow::HandlePointerLeave(void* data, wl_pointer*, std::uint32_t, wl_surface*)
{
    auto* window = static_cast<LWindow*>(data);
    if (window) {
        NeoWPF::Core::SetInputRootForWindow(window);
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
    NeoWPF::Input::InputManager::GetInstance().ProcessMouseInput(
        window->m_pointerX, window->m_pointerY, window->m_pointerButtonState);
}

void LWindow::HandlePointerButton(void* data, wl_pointer*, std::uint32_t, std::uint32_t, std::uint32_t button,
                                 std::uint32_t state)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window) {
        return;
    }
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
    NeoWPF::Input::InputManager::GetInstance().ProcessMouseInput(
        window->m_pointerX, window->m_pointerY, window->m_pointerButtonState);
}

void LWindow::HandlePointerAxis(void* data, wl_pointer*, std::uint32_t, std::uint32_t axis, wl_fixed_t value)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window) {
        return;
    }
    NeoWPF::Core::SetInputRootForWindow(window);
    const int wheelDelta = (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) ? static_cast<int>(wl_fixed_to_double(value) * 120.0) : 0;
    NeoWPF::Input::InputManager::GetInstance().ProcessMouseInput(
        window->m_pointerX, window->m_pointerY, window->m_pointerButtonState, wheelDelta);
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

void LWindow::HandleKeyboardKey(void* data, wl_keyboard*, std::uint32_t, std::uint32_t, std::uint32_t key,
                               std::uint32_t state)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window || !window->m_xkbState) {
        return;
    }

    const auto keysym = xkb_state_key_get_one_sym(window->m_xkbState, key + 8u);
    const auto virtualKey = TranslateXkbKeysymToVirtualKey(keysym);
    if (virtualKey != 0) {
        window->UpdateKeyStateFromWayland(virtualKey, state == WL_KEYBOARD_KEY_STATE_PRESSED);
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

void LWindow::HandleSurfaceConfigure(void* data, xdg_surface*, std::uint32_t serial)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window) {
        return;
    }
    window->m_configureSerial = serial;
    window->m_configured = true;
}

void LWindow::HandleToplevelConfigure(void* data, xdg_toplevel*, int32_t width,
                                      int32_t height, wl_array*)
{
    auto* window = static_cast<LWindow*>(data);
    if (!window) {
        return;
    }
    if (width > 0 && height > 0) {
        window->SetSize(width, height);
    }
}

void LWindow::HandleToplevelClose(void* data, xdg_toplevel*)
{
    auto* window = static_cast<LWindow*>(data);
    if (window) {
        window->m_closed = true;
    }
}

}

namespace NeoWPF::Platform {

void Window::Resize(int width, int height) const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        platform->Windowing().ResizeWindow(m_nativeHandle, width, height);
    }
}

void Window::Show() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        platform->Windowing().ShowWindow(m_nativeHandle);
    }
}

void Window::Hide() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        platform->Windowing().HideWindow(m_nativeHandle);
    }
}

void Window::Close() const noexcept
{
    if (auto* platform = CurrentPlatform()) {
        platform->Windowing().CloseWindow(m_nativeHandle);
    }
}

void Window::SetContent(std::shared_ptr<NeoWPF::UI::Visual> content) noexcept
{
    SetWindowContent(m_nativeHandle, content);
    if (!m_nativeHandle) {
        m_content = std::move(content);
    }
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetContent() const noexcept
{
    if (m_nativeHandle) {
        return GetWindowContent(m_nativeHandle);
    }
    return m_content;
}

void Window::SetTitleBar(std::shared_ptr<NeoWPF::UI::Visual> titleBar) noexcept
{
    SetWindowTitleBar(m_nativeHandle, titleBar);
    if (!m_nativeHandle) {
        m_titleBar = std::move(titleBar);
    }
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetTitleBar() const noexcept
{
    if (m_nativeHandle) {
        return GetWindowTitleBar(m_nativeHandle);
    }
    return m_titleBar;
}

void Window::SetWindowContent(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> content)
{
    auto& app = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(app.windowsMutex);
    for (auto& windowState : app.windows) {
        if (windowState.nativeWindowHandle == nativeHandle && windowState.windowHost) {
            windowState.contentRoot = content.get();
            windowState.windowHost->SetContent(std::move(content));
            return;
        }
    }
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetWindowContent(void* nativeHandle)
{
    auto& app = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(app.windowsMutex);
    for (const auto& windowState : app.windows) {
        if (windowState.nativeWindowHandle == nativeHandle && windowState.windowHost) {
            return windowState.windowHost->GetContent();
        }
    }
    return nullptr;
}

void Window::SetWindowTitleBar(void* nativeHandle, std::shared_ptr<NeoWPF::UI::Visual> titleBar)
{
    auto& app = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(app.windowsMutex);
    for (auto& windowState : app.windows) {
        if (windowState.nativeWindowHandle == nativeHandle && windowState.windowHost) {
            windowState.windowHost->SetTitleBar(std::move(titleBar));
            return;
        }
    }
}

std::shared_ptr<NeoWPF::UI::Visual> Window::GetWindowTitleBar(void* nativeHandle)
{
    auto& app = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(app.windowsMutex);
    for (const auto& windowState : app.windows) {
        if (windowState.nativeWindowHandle == nativeHandle && windowState.windowHost) {
            return windowState.windowHost->GetTitleBar();
        }
    }
    return nullptr;
}

void* Window::CreateWindow(void*, const wchar_t* title, int width, int height)
{
    auto* platform = CurrentPlatform();
    return platform ? platform->Windowing().CreateWindow(title, width, height) : nullptr;
}

}
