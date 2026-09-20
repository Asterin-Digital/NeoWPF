#include <NeoWPF/Core/Threading/BackgroundExecutor.h>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace NeoWPF::Core::Threading {

struct BackgroundExecutor::State {
    std::deque<std::function<void()>> tasks;
    std::vector<std::thread> workers;
    mutable std::mutex mutex;
    std::condition_variable condition;
    bool stopping = false;
    bool cancelPending = true;
};

BackgroundExecutor::BackgroundExecutor(std::size_t workerCount)
    : m_state(std::make_unique<State>())
{
    if (workerCount == 0) {
        workerCount = std::thread::hardware_concurrency();
        if (workerCount == 0) {
            workerCount = 1;
        }
    }

    m_state->workers.reserve(workerCount);
    for (std::size_t index = 0; index < workerCount; ++index) {
        m_state->workers.emplace_back([state = m_state.get()] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(state->mutex);
                    state->condition.wait(lock, [state] {
                        return state->stopping || !state->tasks.empty();
                    });

                    if (state->tasks.empty()) {
                        if (state->stopping) {
                            return;
                        }
                        continue;
                    }

                    task = std::move(state->tasks.front());
                    state->tasks.pop_front();
                }

                try {
                    task();
                } catch (...) {
                }
            }
        });
    }
}

BackgroundExecutor::~BackgroundExecutor()
{
    Stop();
}

bool BackgroundExecutor::Post(std::function<void()> task)
{
    if (!task) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_state->mutex);
        if (m_state->stopping) {
            return false;
        }
        m_state->tasks.push_back(std::move(task));
    }
    m_state->condition.notify_one();
    return true;
}

bool BackgroundExecutor::Post(CancellationToken token, std::function<void(CancellationToken)> task)
{
    if (!task) {
        return false;
    }

    return Post([token, task = std::move(task)]() mutable {
        if (!token.IsCancellationRequested()) {
            task(token);
        }
    });
}

void BackgroundExecutor::Stop(ShutdownMode mode) noexcept
{
    {
        std::lock_guard<std::mutex> lock(m_state->mutex);
        if (!m_state->stopping) {
            m_state->stopping = true;
            m_state->cancelPending = mode == ShutdownMode::CancelPending;
            if (m_state->cancelPending) {
                m_state->tasks.clear();
            }
        }
    }

    m_state->condition.notify_all();
    const auto currentThread = std::this_thread::get_id();
    for (auto& worker : m_state->workers) {
        if (worker.joinable() && worker.get_id() != currentThread) {
            worker.join();
        }
    }
}

bool BackgroundExecutor::IsStopped() const noexcept
{
    std::lock_guard<std::mutex> lock(m_state->mutex);
    return m_state->stopping;
}

std::size_t BackgroundExecutor::GetWorkerCount() const noexcept
{
    return m_state->workers.size();
}

}
