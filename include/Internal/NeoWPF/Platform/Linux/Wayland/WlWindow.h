/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <NeoWPF/Platform/Linux/Wayland/WlWindowing.h>
#include <NeoWPF/Platform/Linux/LKeyboard.h>
#include <fractional-scale-v1-client-protocol.h>

#include <string>
#include <chrono>
#include <vector>

#include <xkbcommon/xkbcommon.h>

namespace NeoWPF::Platform::Linux {

class WlWindow final {
public:
        WlWindow(WlWindowing& windowing, const wchar_t* title, int width, int height,
            bool transparent = false);
    ~WlWindow();

    WlWindow(const WlWindow&) = delete;
    WlWindow& operator=(const WlWindow&) = delete;

    bool IsValid() const noexcept;
    void Close() noexcept;

    wl_surface* Surface() const noexcept;
    wl_display* Display() const noexcept;
    const std::wstring& Title() const noexcept;
    int Width() const noexcept;
    int Height() const noexcept;
    bool IsTransparent() const noexcept;
    void SetSize(int width, int height) noexcept;
    bool IsConfigured() const noexcept;
    bool IsClosed() const noexcept;
    bool IsMaximized() const noexcept;
    double ScaleFactor() const noexcept;
    /*
        @summary
        Explains why no usable scale factor is currently available.

        @internal
    */
    const char* ScaleFactorUnavailableReason() const noexcept;
    /*
        @summary
        Applies a scale reported by the fractional-scale protocol.

        @internal
    */
    void SetFractionalScaleFactor(double scaleFactor) noexcept;
    /*
        @summary
        Recalculates the integer scale from the outputs entered by the surface.

        @internal
    */
    void UpdateIntegerScaleFactor() noexcept;
    /*
        @summary
        Removes an output from the outputs entered by the surface.

        @param output
        Output that was removed.

        @internal
    */
    void RemoveOutput(wl_output* output) noexcept;
    void Minimize() noexcept;
    void Maximize(bool maximize) noexcept;
    void AcknowledgeConfigure() noexcept;
    void AttachInputDevices();
    void ProcessKeyRepeat();

    static void HandleSurfaceConfigure(void* data, xdg_surface* surface, std::uint32_t serial);
    static void HandleToplevelConfigure(void* data, xdg_toplevel* toplevel, int32_t width,
                                        int32_t height, wl_array* states);
    static void HandleToplevelClose(void* data, xdg_toplevel* toplevel);
    /*
        @summary
        Tracks an output entered by the surface.

        @internal
    */
    static void HandleSurfaceEnter(void* data, wl_surface* surface, wl_output* output);
    /*
        @summary
        Tracks an output left by the surface.

        @internal
    */
    static void HandleSurfaceLeave(void* data, wl_surface* surface, wl_output* output);

    static void HandlePointerEnter(void* data, wl_pointer* pointer, std::uint32_t serial,
                                   wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
    static void HandlePointerLeave(void* data, wl_pointer* pointer, std::uint32_t serial,
                                   wl_surface* surface);
    static void HandlePointerMotion(void* data, wl_pointer* pointer, std::uint32_t time,
                                   wl_fixed_t sx, wl_fixed_t sy);
    static void HandlePointerButton(void* data, wl_pointer* pointer, std::uint32_t serial,
                                   std::uint32_t time, std::uint32_t button, std::uint32_t state);
    static void HandlePointerAxis(void* data, wl_pointer* pointer, std::uint32_t time,
                                 std::uint32_t axis, wl_fixed_t value);

    static void HandleKeyboardKeymap(void* data, wl_keyboard* keyboard, std::uint32_t format,
                                     int fd, std::uint32_t size);
    static void HandleKeyboardEnter(void* data, wl_keyboard* keyboard, std::uint32_t serial,
                                   wl_surface* surface, wl_array* keys);
    static void HandleKeyboardLeave(void* data, wl_keyboard* keyboard, std::uint32_t serial,
                                   wl_surface* surface);
    static void HandleKeyboardKey(void* data, wl_keyboard* keyboard, std::uint32_t serial,
                                 std::uint32_t time, std::uint32_t key, std::uint32_t state);
    static void HandleKeyboardModifiers(void* data, wl_keyboard* keyboard, std::uint32_t serial,
                                       std::uint32_t mods_depressed, std::uint32_t mods_latched,
                                       std::uint32_t mods_locked, std::uint32_t group);

private:
    void UpdateKeyStateFromWayland(std::uint32_t key, bool isPressed, wchar_t charCode);
    std::uint32_t WaylandButtonToNeoWPFButton(std::uint32_t button) const noexcept;

    WlWindowing& m_windowing;
    wl_display* m_display = nullptr;
    wl_surface* m_surface = nullptr;
    wl_seat* m_seat = nullptr;
    wl_pointer* m_pointer = nullptr;
    wl_keyboard* m_keyboard = nullptr;
    xdg_surface* m_xdgSurface = nullptr;
    xdg_toplevel* m_xdgToplevel = nullptr;
    wp_fractional_scale_v1* m_fractionalScale = nullptr;
    std::vector<wl_output*> m_outputs;
    xkb_context* m_xkbContext = nullptr;
    xkb_keymap* m_xkbKeymap = nullptr;
    xkb_state* m_xkbState = nullptr;
    std::wstring m_title;
    int m_width = 0;
    int m_height = 0;
    double m_pointerX = 0.0;
    double m_pointerY = 0.0;
    std::uint32_t m_pointerButtonState = 0;
    std::uint32_t m_configureSerial = 0;
    bool m_configured = false;
    bool m_closed = false;
    bool m_maximized = false;
    bool m_transparent = false;
    bool m_hasFractionalScale = false;
    double m_scaleFactor = 0.0;
    std::uint32_t m_repeatKey = 0;
    bool m_repeatActive = false;
    std::chrono::steady_clock::time_point m_repeatNext{};
};

}
