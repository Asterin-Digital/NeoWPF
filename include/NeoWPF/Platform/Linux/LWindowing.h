#pragma once

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

#include <cstdint>

namespace NeoWPF::Platform::Linux {

class LWindowing final {
public:
    LWindowing() = default;
    ~LWindowing();

    LWindowing(const LWindowing&) = delete;
    LWindowing& operator=(const LWindowing&) = delete;

    bool Connect();
    void Disconnect() noexcept;
    bool IsConnected() const noexcept;

    wl_display* Display() const noexcept;
    wl_compositor* Compositor() const noexcept;
    wl_shm* Shm() const noexcept;
    xdg_wm_base* XdgWmBase() const noexcept;
    bool HasXdgShell() const noexcept;

    static void HandleGlobal(void* data, wl_registry* registry, std::uint32_t name,
                             const char* interface, std::uint32_t version);
    static void HandleGlobalRemove(void* data, wl_registry* registry, std::uint32_t name);

    wl_display* m_display = nullptr;
    wl_registry* m_registry = nullptr;
    wl_compositor* m_compositor = nullptr;
    wl_shm* m_shm = nullptr;
    xdg_wm_base* m_xdgWmBase = nullptr;
    bool m_hasXdgShell = false;
};

}