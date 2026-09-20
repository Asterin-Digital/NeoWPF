#include <NeoWPF/Core/Threading/MainThreadExecutor.h>

#include <deque>
#include <mutex>
#include <thread>
#include <utility>

namespace NeoWPF::Core::Threading {

struct MainThreadExecutor::State {
    std::thread::id ownerThread;
    std::deque<std::function<void()>> tasks;
    mutable std::mutex mutex;
    bool stopped = false;
};

MainThreadExecutor::MainThreadExecutor()
    : m_state(std::make_shared<State>())
{
    m_state->ownerThread = std::this_thread::get_id();
}

MainThreadExecutor::~MainThreadExecutor()
{
    Stop();
}

bool MainThreadExecutor::Post(std::function<void()> task)
{
    return GetHandle().Post(std::move(task));
}

MainThreadExecutor::Handle::Handle(const std::shared_ptr<State>& state) noexcept
    : m_state(state)
{
}

bool MainThreadExecutor::Handle::Post(std::function<void()> task) const
{
    if (!task) {
        return false;
    }

    auto state = m_state.lock();
    if (!state) {
        return false;
    }

    std::lock_guard<std::mutex> lock(state->mutex);
    if (state->stopped) {
        return false;
    }

    state->tasks.push_back(std::move(task));
    return true;
}

MainThreadExecutor::Handle MainThreadExecutor::GetHandle() const noexcept
{
    return Handle(m_state);
}

std::size_t MainThreadExecutor::Drain(std::chrono::milliseconds budget)
{
    if (!IsCurrentThread()) {
        return 0;
    }

    const bool hasDeadline = budget != std::chrono::milliseconds::max();
    const auto deadline = std::chrono::steady_clock::now() + budget;
    std::size_t executed = 0;
    while (true) {
        std::function<void()> task;
        {
            std::lock_guard<std::mutex> lock(m_state->mutex);
            if (m_state->tasks.empty()) {
                break;
            }
            task = std::move(m_state->tasks.front());
            m_state->tasks.pop_front();
        }

        task();
        ++executed;

        if (hasDeadline && std::chrono::steady_clock::now() >= deadline) {
            break;
        }
    }

    return executed;
}

bool MainThreadExecutor::IsCurrentThread() const noexcept
{
    return std::this_thread::get_id() == m_state->ownerThread;
}

bool MainThreadExecutor::IsStopped() const noexcept
{
    std::lock_guard<std::mutex> lock(m_state->mutex);
    return m_state->stopped;
}

void MainThreadExecutor::Stop(ShutdownMode mode) noexcept
{
    if (mode == ShutdownMode::DrainPending && IsCurrentThread()) {
        try {
            Drain(std::chrono::milliseconds::max());
        } catch (...) {
            std::lock_guard<std::mutex> lock(m_state->mutex);
            m_state->tasks.clear();
        }
    }

    std::lock_guard<std::mutex> lock(m_state->mutex);
    m_state->stopped = true;
    m_state->tasks.clear();
}

}
