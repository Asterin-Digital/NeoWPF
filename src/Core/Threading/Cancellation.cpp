#include <NeoWPF/Core/Threading/Cancellation.h>

#include <atomic>

namespace NeoWPF::Core::Threading {

struct CancellationToken::State {
    std::atomic<bool> requested{false};
};

CancellationToken::CancellationToken(std::shared_ptr<State> state) noexcept
    : m_state(std::move(state))
{
}

bool CancellationToken::IsCancellationRequested() const noexcept
{
    return m_state && m_state->requested.load(std::memory_order_acquire);
}

CancellationSource::CancellationSource()
    : m_state(std::make_shared<CancellationToken::State>())
{
}

CancellationToken CancellationSource::GetToken() const noexcept
{
    return CancellationToken(m_state);
}

bool CancellationSource::IsCancellationRequested() const noexcept
{
    return GetToken().IsCancellationRequested();
}

bool CancellationSource::RequestCancellation() noexcept
{
    bool expected = false;
    return m_state->requested.compare_exchange_strong(
        expected,
        true,
        std::memory_order_release,
        std::memory_order_relaxed);
}

}
