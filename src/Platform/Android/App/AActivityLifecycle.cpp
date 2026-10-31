#include <NeoWPF/Platform/Android/AActivityLifecycle.h>

namespace NeoWPF::Platform::Android {

bool AActivityLifecycle::DispatchInputQueueCreated(InputQueueCallback callback,
                                                 void* context,
                                                 void* inputQueue) noexcept
{
    if (!callback || !inputQueue || !BeginCallback(true)) {
        return false;
    }

    callback(context, inputQueue);
    EndCallback();
    return true;
}

bool AActivityLifecycle::DispatchInputQueueDestroyed(InputQueueCallback callback,
                                                    void* context,
                                                    void* inputQueue) noexcept
{
    if (!callback || !inputQueue || !BeginCallback(false)) {
        return false;
    }

    callback(context, inputQueue);
    EndCallback();
    return true;
}

bool AActivityLifecycle::DispatchNativeWindow(
    SurfaceAvailableCallback callback, void* context, void* nativeWindow,
    const ANativeWindowOperations& operations) noexcept
{
    if (!callback || !nativeWindow || !operations.acquire ||
        !operations.release || !operations.width || !operations.height) {
        return false;
    }

    operations.acquire(nativeWindow);
    const int nativeWidth = operations.width(nativeWindow);
    const int nativeHeight = operations.height(nativeWindow);
    const bool dispatched = DispatchSurfaceAvailable(
        callback, context, nativeWindow,
        nativeWidth > 0 ? static_cast<unsigned int>(nativeWidth) : 0u,
        nativeHeight > 0 ? static_cast<unsigned int>(nativeHeight) : 0u);
    if (!dispatched) {
        operations.release(nativeWindow);
    }
    return dispatched;
}

bool AActivityLifecycle::DispatchSurfaceAvailable(
    SurfaceAvailableCallback callback, void* context, void* nativeWindow,
    unsigned int width, unsigned int height) noexcept
{
    if (!callback || !BeginCallback(true)) {
        return false;
    }

    callback(context, nativeWindow, width, height);
    EndCallback();
    return true;
}

bool AActivityLifecycle::DispatchSurfaceDestroyed(
    SurfaceDestroyedCallback callback, void* context) noexcept
{
    if (!callback || !BeginCallback(false)) {
        return false;
    }

    callback(context);
    EndCallback();
    return true;
}

bool AActivityLifecycle::CompleteApplication(
    void (*finishActivity)(void*) noexcept, void* context) noexcept
{
    if (!finishActivity) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_state != State::Active) {
            return false;
        }

        m_state = State::ApplicationCompleted;
        ++m_inFlightCallbacks;
    }

    finishActivity(context);
    EndCallback();
    return true;
}

bool AActivityLifecycle::MarkActivityDestroyed() noexcept
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_state == State::ActivityDestroyed ||
        m_state == State::ActivityDestroying) {
        return false;
    }

    m_state = State::ActivityDestroying;
    m_callbacksFinished.wait(lock, [this] {
        return m_inFlightCallbacks == 0;
    });
    m_state = State::ActivityDestroyed;
    return true;
}

bool AActivityLifecycle::IsActivityDestroying() const noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::ActivityDestroying ||
        m_state == State::ActivityDestroyed;
}

bool AActivityLifecycle::BeginCallback(bool requireActive) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_state == State::ActivityDestroying ||
        m_state == State::ActivityDestroyed ||
        (requireActive && m_state != State::Active)) {
        return false;
    }

    ++m_inFlightCallbacks;
    return true;
}

void AActivityLifecycle::EndCallback() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    --m_inFlightCallbacks;
    if (m_inFlightCallbacks == 0) {
        m_callbacksFinished.notify_all();
    }
}

}