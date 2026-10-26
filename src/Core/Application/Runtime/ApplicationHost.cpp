#include <NeoWPF/Core/Application/ApplicationHost.h>

#include <utility>

namespace NeoWPF::Core {

bool SessionLifecycle::Activate() noexcept
{
    State state = m_sharedState->state.load(std::memory_order_acquire);
    while (state != State::Shutdown) {
        if (state == State::Active ||
            m_sharedState->state.compare_exchange_weak(
                state,
                State::Active,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
            return true;
        }
    }
    return false;
}

void SessionLifecycle::Deactivate() noexcept
{
    State active = State::Active;
    m_sharedState->state.compare_exchange_strong(
        active,
        State::Inactive,
        std::memory_order_acq_rel,
        std::memory_order_acquire);
}

void SessionLifecycle::Shutdown() noexcept
{
    m_sharedState->state.store(State::Shutdown, std::memory_order_release);
}

void SessionLifecycle::Begin() noexcept
{
    m_sharedState->state.store(State::Inactive, std::memory_order_release);
}

bool SessionLifecycle::IsActive() const noexcept
{
    return m_sharedState->state.load(std::memory_order_acquire) == State::Active;
}

bool SessionLifecycle::IsShutdown() const noexcept
{
    return m_sharedState->state.load(std::memory_order_acquire) == State::Shutdown;
}

ApplicationSession::ApplicationSession(std::shared_ptr<NeoWPF::UI::Window> contentHost) noexcept
    : m_contentHost(std::move(contentHost))
{
}

bool ApplicationSession::Activate() noexcept
{
    return m_lifecycle.Activate();
}

void ApplicationSession::Deactivate() noexcept
{
    m_lifecycle.Deactivate();
}

void ApplicationSession::Shutdown() noexcept
{
    m_lifecycle.Shutdown();
}

bool ApplicationSession::IsActive() const noexcept
{
    return m_lifecycle.IsActive();
}

bool ApplicationSession::IsShutdown() const noexcept
{
    return m_lifecycle.IsShutdown();
}

void ApplicationSession::Begin() noexcept
{
    m_lifecycle.Begin();
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

}