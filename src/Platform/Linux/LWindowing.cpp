#include <NeoWPF/Platform/Linux/LWindowing.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace NeoWPF::Platform::Linux {
namespace {

const wl_registry_listener RegistryListener = {
    &LWindowing::HandleGlobal,
    &LWindowing::HandleGlobalRemove,
};

void HandleXdgWmBasePing(void* data, xdg_wm_base* shell, std::uint32_t serial)
{
    auto* windowing = static_cast<LWindowing*>(data);
    xdg_wm_base_pong(shell, serial);
    if (windowing->Display()) {
        wl_display_flush(windowing->Display());
    }
}

const xdg_wm_base_listener XdgWmBaseListener = {
    &HandleXdgWmBasePing,
};

}

LWindowing::~LWindowing()
{
    Disconnect();
}

bool LWindowing::Connect()
{
    if (IsConnected()) {
        return true;
    }

    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        return false;
    }

    m_registry = wl_display_get_registry(m_display);
    if (!m_registry) {
        Disconnect();
        return false;
    }

    if (wl_registry_add_listener(m_registry, &RegistryListener, this) != 0 ||
        wl_display_roundtrip(m_display) < 0) {
        Disconnect();
        return false;
    }

    return m_compositor != nullptr && m_hasXdgShell;
}

void LWindowing::Disconnect() noexcept
{
    if (m_compositor) {
        wl_compositor_destroy(m_compositor);
        m_compositor = nullptr;
    }
    if (m_shm) {
        wl_shm_destroy(m_shm);
        m_shm = nullptr;
    }
    if (m_xdgWmBase) {
        xdg_wm_base_destroy(m_xdgWmBase);
        m_xdgWmBase = nullptr;
    }
    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }
    if (m_display) {
        wl_display_disconnect(m_display);
        m_display = nullptr;
    }
    m_hasXdgShell = false;
}

bool LWindowing::IsConnected() const noexcept
{
    return m_display != nullptr && m_registry != nullptr;
}

wl_display* LWindowing::Display() const noexcept
{
    return m_display;
}

wl_compositor* LWindowing::Compositor() const noexcept
{
    return m_compositor;
}

wl_shm* LWindowing::Shm() const noexcept
{
    return m_shm;
}

xdg_wm_base* LWindowing::XdgWmBase() const noexcept
{
    return m_xdgWmBase;
}

bool LWindowing::HasXdgShell() const noexcept
{
    return m_hasXdgShell;
}

void LWindowing::HandleGlobal(void* data, wl_registry* registry, std::uint32_t name,
                              const char* interface, std::uint32_t version)
{
    auto* windowing = static_cast<LWindowing*>(data);
    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        const auto bindVersion = std::min(version, 4u);
        windowing->m_compositor = static_cast<wl_compositor*>(wl_registry_bind(
            registry, name, &wl_compositor_interface, bindVersion));
    } else if (std::strcmp(interface, wl_shm_interface.name) == 0) {
        windowing->m_shm = static_cast<wl_shm*>(wl_registry_bind(
            registry, name, &wl_shm_interface, 1));
    } else if (std::strcmp(interface, "xdg_wm_base") == 0) {
        windowing->m_xdgWmBase = static_cast<xdg_wm_base*>(wl_registry_bind(
            registry, name, &xdg_wm_base_interface, 1));
        if (windowing->m_xdgWmBase) {
            xdg_wm_base_add_listener(windowing->m_xdgWmBase, &XdgWmBaseListener, windowing);
            windowing->m_hasXdgShell = true;
        }
    }
}

void LWindowing::HandleGlobalRemove(void*, wl_registry*, std::uint32_t)
{
}

}