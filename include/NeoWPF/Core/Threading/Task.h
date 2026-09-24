/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Core/Threading/MainThreadExecutor.h>

#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace NeoWPF::Core::Threading {

template <typename Result>
class Task {
private:
    struct State {
        std::mutex mutex;
        std::condition_variable condition;
        std::optional<Result> result;
        std::exception_ptr exception;
        std::vector<std::function<void()>> continuations;
        bool completed = false;
    };

public:
    Task() = default;

    Result Get() const
    {
        Wait();
        std::lock_guard<std::mutex> lock(m_state->mutex);
        if (m_state->exception) {
            std::rethrow_exception(m_state->exception);
        }
        return *m_state->result;
    }

    void Wait() const
    {
        if (!m_state) {
            throw std::runtime_error("Task is empty");
        }
        std::unique_lock<std::mutex> lock(m_state->mutex);
        m_state->condition.wait(lock, [state = m_state] { return state->completed; });
    }

    bool IsCompleted() const noexcept
    {
        if (!m_state) {
            return false;
        }
        std::lock_guard<std::mutex> lock(m_state->mutex);
        return m_state->completed;
    }

    template <typename Function>
    bool Then(MainThreadExecutor& executor, Function&& function) const
    {
        if (!m_state) {
            return false;
        }

        using Callback = std::decay_t<Function>;
        auto callback = std::make_shared<Callback>(std::forward<Function>(function));
        auto handle = executor.GetHandle();
        std::function<void()> continuation = [state = m_state, handle, callback]() mutable {
            handle.Post([state, callback]() mutable {
                std::optional<Result> result;
                {
                    std::lock_guard<std::mutex> lock(state->mutex);
                    if (state->exception || !state->result) {
                        return;
                    }
                    result = *state->result;
                }
                (*callback)(*result);
            });
        };

        std::unique_lock<std::mutex> lock(m_state->mutex);
        if (m_state->completed) {
            lock.unlock();
            continuation();
            return true;
        }
        m_state->continuations.push_back(std::move(continuation));
        return true;
    }

private:
    explicit Task(std::shared_ptr<State> state)
        : m_state(std::move(state))
    {
    }

    void SetResult(Result result) const
    {
        Complete([&result](State& state) { state.result = std::move(result); });
    }

    void SetException(std::exception_ptr exception) const
    {
        Complete([&exception](State& state) { state.exception = exception; });
    }

    template <typename Setter>
    void Complete(Setter&& setter) const
    {
        std::vector<std::function<void()>> continuations;
        {
            std::lock_guard<std::mutex> lock(m_state->mutex);
            if (m_state->completed) {
                return;
            }
            setter(*m_state);
            m_state->completed = true;
            continuations = std::move(m_state->continuations);
        }
        for (auto& continuation : continuations) {
            continuation();
        }
        m_state->condition.notify_all();
    }

    std::shared_ptr<State> m_state;
    friend class BackgroundExecutor;
};

template <>
class Task<void> {
private:
    struct State {
        std::mutex mutex;
        std::condition_variable condition;
        std::exception_ptr exception;
        std::vector<std::function<void()>> continuations;
        bool completed = false;
    };

public:
    Task() = default;

    void Get() const
    {
        Wait();
        std::lock_guard<std::mutex> lock(m_state->mutex);
        if (m_state->exception) {
            std::rethrow_exception(m_state->exception);
        }
    }

    void Wait() const
    {
        if (!m_state) {
            throw std::runtime_error("Task is empty");
        }
        std::unique_lock<std::mutex> lock(m_state->mutex);
        m_state->condition.wait(lock, [state = m_state] { return state->completed; });
    }

    bool IsCompleted() const noexcept
    {
        if (!m_state) {
            return false;
        }
        std::lock_guard<std::mutex> lock(m_state->mutex);
        return m_state->completed;
    }

    template <typename Function>
    bool Then(MainThreadExecutor& executor, Function&& function) const
    {
        if (!m_state) {
            return false;
        }

        using Callback = std::decay_t<Function>;
        auto callback = std::make_shared<Callback>(std::forward<Function>(function));
        auto handle = executor.GetHandle();
        std::function<void()> continuation = [state = m_state, handle, callback]() mutable {
            handle.Post([state, callback]() mutable {
                {
                    std::lock_guard<std::mutex> lock(state->mutex);
                    if (state->exception) {
                        return;
                    }
                }
                (*callback)();
            });
        };

        std::unique_lock<std::mutex> lock(m_state->mutex);
        if (m_state->completed) {
            lock.unlock();
            continuation();
            return true;
        }
        m_state->continuations.push_back(std::move(continuation));
        return true;
    }

private:
    explicit Task(std::shared_ptr<State> state)
        : m_state(std::move(state))
    {
    }

    void SetResult() const
    {
        Complete([](State&) {});
    }

    void SetException(std::exception_ptr exception) const
    {
        Complete([&exception](State& state) { state.exception = exception; });
    }

    template <typename Setter>
    void Complete(Setter&& setter) const
    {
        std::vector<std::function<void()>> continuations;
        {
            std::lock_guard<std::mutex> lock(m_state->mutex);
            if (m_state->completed) {
                return;
            }
            setter(*m_state);
            m_state->completed = true;
            continuations = std::move(m_state->continuations);
        }
        for (auto& continuation : continuations) {
            continuation();
        }
        m_state->condition.notify_all();
    }

    std::shared_ptr<State> m_state;
    friend class BackgroundExecutor;
};

}
