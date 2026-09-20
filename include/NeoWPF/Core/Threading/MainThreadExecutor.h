/*
MIT License — Copyright (c) 2026 NeoWPF
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
public:
    MainThreadExecutor();
    ~MainThreadExecutor();

    MainThreadExecutor(const MainThreadExecutor&) = delete;
    MainThreadExecutor& operator=(const MainThreadExecutor&) = delete;

    bool Post(std::function<void()> task);
    std::size_t Drain(std::chrono::milliseconds budget);
    bool IsCurrentThread() const noexcept;
    bool IsStopped() const noexcept;
    void Stop(ShutdownMode mode = ShutdownMode::CancelPending) noexcept;

private:
    struct State;
    std::unique_ptr<State> m_state;
};

}
