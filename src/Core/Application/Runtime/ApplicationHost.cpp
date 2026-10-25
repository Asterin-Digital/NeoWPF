#include <NeoWPF/Core/Application/ApplicationHost.h>

#include <utility>

namespace NeoWPF::Core {

ApplicationSession::ApplicationSession(std::shared_ptr<NeoWPF::UI::Window> contentHost) noexcept
    : m_contentHost(std::move(contentHost))
{
}

bool ApplicationSession::Activate() noexcept
{
    State state = m_state.load(std::memory_order_acquire);
    while (state != State::Shutdown) {
        if (state == State::Active ||
            m_state.compare_exchange_weak(
                state,
                State::Active,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
            return true;
        }
    }
    return false;
}

void ApplicationSession::Deactivate() noexcept
{
    State active = State::Active;
    m_state.compare_exchange_strong(
        active,
        State::Inactive,
        std::memory_order_acq_rel,
        std::memory_order_acquire);
}

void ApplicationSession::Shutdown() noexcept
{
    m_state.store(State::Shutdown, std::memory_order_release);
    (void)DetachSurface();
}

bool ApplicationSession::IsActive() const noexcept
{
    return m_state.load(std::memory_order_acquire) == State::Active;
}

bool ApplicationSession::IsShutdown() const noexcept
{
    return m_state.load(std::memory_order_acquire) == State::Shutdown;
}

void ApplicationSession::Begin() noexcept
{
    m_state.store(State::Inactive, std::memory_order_release);
}

void ApplicationSession::SetContentHost(std::shared_ptr<NeoWPF::UI::Window> contentHost) noexcept
{
    m_contentHost = std::move(contentHost);
}

const std::shared_ptr<NeoWPF::UI::Window>& ApplicationSession::GetContentHost() const noexcept
{
    return m_contentHost;
}

std::shared_ptr<NeoWPF::UI::Window> ApplicationSession::SelectContentHost(
    const std::shared_ptr<NeoWPF::UI::Window>& explicitHost,
    bool isPrimary) const noexcept
{
    if (explicitHost) {
        return explicitHost;
    }
    return isPrimary ? m_contentHost : nullptr;
}

bool ApplicationSession::AttachSurface(const NeoWPF::Rendering::RenderSurface& surface)
{
    if (!surface.IsValid()) {
        return false;
    }
    std::lock_guard<std::mutex> lock(m_surfaceMutex);
    if (IsShutdown() || m_surface) {
        return false;
    }
    m_surface = surface;
    return true;
}

std::optional<NeoWPF::Rendering::RenderSurface> ApplicationSession::DetachSurface()
{
    std::lock_guard<std::mutex> lock(m_surfaceMutex);
    if (!m_surface) {
        return std::nullopt;
    }
    auto surface = std::move(m_surface);
    m_surface.reset();
    surface->Invalidate();
    return surface;
}

std::optional<NeoWPF::Rendering::RenderSurface> ApplicationSession::GetSurface() const
{
    std::lock_guard<std::mutex> lock(m_surfaceMutex);
    return m_surface;
}

}