/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>

namespace NeoWPF::Core::Threading {

enum class ShutdownMode {
    CancelPending,
    DrainPending,
};

class MainThreadExecutor {
private:
    struct State;

public:
    class Handle {
    public:
        bool Post(std::function<void()> task) const;

    private:
        explicit Handle(const std::shared_ptr<State>& state) noexcept;
        std::weak_ptr<State> m_state;
        friend class MainThreadExecutor;
    };

    MainThreadExecutor();
    ~MainThreadExecutor();

    MainThreadExecutor(const MainThreadExecutor&) = delete;
    MainThreadExecutor& operator=(const MainThreadExecutor&) = delete;

    bool Post(std::function<void()> task);
    Handle GetHandle() const noexcept;
    std::size_t Drain(std::chrono::milliseconds budget);
    bool IsCurrentThread() const noexcept;
    bool IsStopped() const noexcept;
    void Stop(ShutdownMode mode = ShutdownMode::CancelPending) noexcept;

private:
    std::shared_ptr<State> m_state;
};

}
