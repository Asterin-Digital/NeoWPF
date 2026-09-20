#include <NeoWPF/Core/Threading/ThreadAffinity.h>

#include <mutex>

namespace NeoWPF::Core::Threading {
namespace {

std::mutex& AffinityMutex() noexcept
{
    static std::mutex mutex;
    return mutex;
}

ThreadId& MainThreadId() noexcept
{
    static ThreadId threadId;
    return threadId;
}

ThreadId& RenderThreadId() noexcept
{
    static ThreadId threadId;
    return threadId;
}

}

ThreadId ThreadAffinity::GetCurrentThreadId() noexcept
{
    return std::this_thread::get_id();
}

ThreadKind ThreadAffinity::GetCurrentThreadKind() noexcept
{
    const ThreadId currentThread = GetCurrentThreadId();
    std::lock_guard<std::mutex> lock(AffinityMutex());

    if (currentThread == MainThreadId()) {
        return ThreadKind::Main;
    }
    if (currentThread == RenderThreadId()) {
        return ThreadKind::Render;
    }
    return ThreadKind::Unknown;
}

void ThreadAffinity::RegisterMainThread() noexcept
{
    std::lock_guard<std::mutex> lock(AffinityMutex());
    MainThreadId() = GetCurrentThreadId();
}

void ThreadAffinity::RegisterRenderThread() noexcept
{
    std::lock_guard<std::mutex> lock(AffinityMutex());
    RenderThreadId() = GetCurrentThreadId();
}

void ThreadAffinity::ClearMainThread() noexcept
{
    std::lock_guard<std::mutex> lock(AffinityMutex());
    MainThreadId() = ThreadId{};
}

void ThreadAffinity::ClearRenderThread() noexcept
{
    std::lock_guard<std::mutex> lock(AffinityMutex());
    RenderThreadId() = ThreadId{};
}

bool ThreadAffinity::IsMainThread() noexcept
{
    return GetCurrentThreadKind() == ThreadKind::Main;
}

bool ThreadAffinity::IsMainThreadRegistered() noexcept
{
    std::lock_guard<std::mutex> lock(AffinityMutex());
    return MainThreadId() != ThreadId{};
}

bool ThreadAffinity::IsRenderThread() noexcept
{
    return GetCurrentThreadKind() == ThreadKind::Render;
}

}