/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <vector>
#include <memory>

namespace NeoWPF::Core::Threading {

class AsyncTaskQueue {
public:
    static AsyncTaskQueue& GetInstance() noexcept {
        static AsyncTaskQueue instance;
        return instance;
    }

    void Enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(task));
        }
        m_condition.notify_one();
    }

    void Shutdown() noexcept {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = false;
        }
        m_condition.notify_all();
        for (auto& thread : m_threads) {
            if (thread.joinable()) thread.join();
        }
    }

    ~AsyncTaskQueue() {
        if (m_running) Shutdown();
    }

private:
    AsyncTaskQueue() noexcept : m_running(true) {
        for (int i = 0; i < 2; ++i) {
            m_threads.emplace_back([this]() { ThreadMain(); });
        }
    }

    void ThreadMain() noexcept {
        while (m_running) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_condition.wait(lock, [this]() { return !m_queue.empty() || !m_running; });
                if (!m_running) break;
                if (m_queue.empty()) continue;
                task = std::move(m_queue.front());
                m_queue.pop();
            }
            if (task) task();
        }
    }

    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_running;
};

}
