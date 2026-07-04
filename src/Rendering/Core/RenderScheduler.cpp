#include <NeoWPF/Rendering/Renderer.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

using namespace NeoWPF::Rendering;

class RenderScheduler {
public:
    RenderScheduler() = default;
    ~RenderScheduler() { Stop(); }

    void Start(HWND hwnd)
    {
        NeoWPF::Rendering::Renderer_Initialize(hwnd);
        m_running = true;
        m_thread = std::thread([this]{ ThreadMain(); });
    }

    void Stop()
    {
        m_running = false;
        m_cv.notify_all();
        if (m_thread.joinable()) m_thread.join();
        NeoWPF::Rendering::Renderer_Shutdown();
    }

    void Submit(CommandList cmd)
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_queue.push(std::move(cmd));
        m_cv.notify_one();
    }

private:
    void ThreadMain()
    {
        while (m_running) {
            CommandList cmd;
            {
                std::unique_lock<std::mutex> lk(m_mutex);
                m_cv.wait(lk, [this]{ return !m_queue.empty() || !m_running; });
                if (!m_running) break;
                cmd = std::move(m_queue.front()); m_queue.pop();
            }
            NeoWPF::Rendering::Renderer_BeginFrame();
            NeoWPF::Rendering::Renderer_Execute(cmd);
            NeoWPF::Rendering::Renderer_Present();
        }
    }

    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<CommandList> m_queue;
    std::atomic<bool> m_running{false};
};
