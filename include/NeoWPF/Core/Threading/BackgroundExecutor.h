/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Core/Threading/MainThreadExecutor.h>
#include <NeoWPF/Core/Threading/Cancellation.h>
#include <NeoWPF/Core/Threading/Task.h>

#include <cstddef>
#include <exception>
#include <future>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace NeoWPF::Core::Threading {

class BackgroundExecutor {
public:
    explicit BackgroundExecutor(std::size_t workerCount = 0);
    ~BackgroundExecutor();

    BackgroundExecutor(const BackgroundExecutor&) = delete;
    BackgroundExecutor& operator=(const BackgroundExecutor&) = delete;

    bool Post(std::function<void()> task);

    bool Post(CancellationToken token, std::function<void(CancellationToken)> task);

    template <typename Function>
    auto SubmitTask(Function&& function)
        -> Task<std::invoke_result_t<Function>>
    {
        using Result = std::invoke_result_t<Function>;
        auto state = std::make_shared<typename Task<Result>::State>();
        Task<Result> task(state);
        if (!Post([task, function = std::forward<Function>(function)]() mutable {
                try {
                    if constexpr (std::is_void_v<Result>) {
                        function();
                        task.SetResult();
                    } else {
                        task.SetResult(function());
                    }
                } catch (...) {
                    task.SetException(std::current_exception());
                }
            })) {
            task.SetException(std::make_exception_ptr(std::runtime_error("BackgroundExecutor is stopped")));
        }
        return task;
    }

    template <typename Function>
    auto Submit(Function&& function)
        -> std::future<std::invoke_result_t<Function>>
    {
        using Result = std::invoke_result_t<Function>;
        auto task = std::make_shared<std::packaged_task<Result()>>(
            std::forward<Function>(function));
        auto result = task->get_future();
        if (!Post([task] { (*task)(); })) {
            std::promise<Result> rejected;
            rejected.set_exception(std::make_exception_ptr(std::runtime_error("BackgroundExecutor is stopped")));
            return rejected.get_future();
        }
        return result;
    }

    template <typename Function>
    auto Submit(CancellationToken token, Function&& function)
        -> std::future<std::invoke_result_t<Function, CancellationToken>>
    {
        using Result = std::invoke_result_t<Function, CancellationToken>;
        auto task = std::make_shared<std::packaged_task<Result()>>(
            [token, function = std::forward<Function>(function)]() mutable {
                return function(token);
            });
        auto result = task->get_future();
        if (!Post([task] { (*task)(); })) {
            std::promise<Result> rejected;
            rejected.set_exception(std::make_exception_ptr(std::runtime_error("BackgroundExecutor is stopped")));
            return rejected.get_future();
        }
        return result;
    }

    void Stop(ShutdownMode mode = ShutdownMode::CancelPending) noexcept;
    bool IsStopped() const noexcept;
    std::size_t GetWorkerCount() const noexcept;

private:
    struct State;
    std::unique_ptr<State> m_state;
};

}
