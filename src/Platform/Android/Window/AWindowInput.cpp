#include <NeoWPF/Platform/Android/AWindowing.h>
#include <NeoWPF/Platform/Android/AActivityInputQueue.h>

namespace NeoWPF::Platform::Android {
namespace {

int GetInputQueueEvent(void* queue, void** outEvent) noexcept
{
    AInputEvent* event = nullptr;
    const int result = AInputQueue_getEvent(static_cast<AInputQueue*>(queue), &event);
    *outEvent = event;
    return result;
}

int PreDispatchInputQueueEvent(void* queue, void* event) noexcept
{
    auto* const inputEvent = static_cast<AInputEvent*>(event);
    if (AInputEvent_getType(inputEvent) != AINPUT_EVENT_TYPE_KEY) {
        return 0;
    }
    return AInputQueue_preDispatchEvent(
        static_cast<AInputQueue*>(queue), inputEvent);
}

void FinishInputQueueEvent(void* queue, void* event, int handled) noexcept
{
    AInputQueue_finishEvent(static_cast<AInputQueue*>(queue),
                            static_cast<AInputEvent*>(event), handled);
}

constexpr AActivityInputQueueOperations InputQueueOperations{
    &GetInputQueueEvent,
    &PreDispatchInputQueueEvent,
    &FinishInputQueueEvent,
};

}

void AWindowing::ProcessInputQueueEvent(Event& event) noexcept
{
    if (event.type == EventType::InputQueueCreated) {
        if (!event.inputQueue || !m_looper || m_inputQueue == event.inputQueue) {
            return;
        }
        if (m_inputQueue) {
            AInputQueue_detachLooper(m_inputQueue);
        }
        AInputQueue_attachLooper(event.inputQueue, m_looper, 0,
                                 &AWindowing::InputQueueLooperCallback, this);
        m_inputQueue = event.inputQueue;
        return;
    }

    if (event.type == EventType::InputQueueDestroyed && event.inputQueue &&
        m_inputQueue == event.inputQueue) {
        AInputQueue_detachLooper(m_inputQueue);
        m_inputQueue = nullptr;
    }
}

void AWindowing::StopInputQueue() noexcept
{
    if (!m_inputQueue) {
        return;
    }
    AInputQueue_detachLooper(m_inputQueue);
    m_inputQueue = nullptr;
}

int AWindowing::InputQueueLooperCallback(int, int, void* context) noexcept
{
    auto* const windowing = static_cast<AWindowing*>(context);
    if (windowing && windowing->m_inputQueue) {
        ADrainUnhandledInputQueue(windowing->m_inputQueue, InputQueueOperations);
    }
    return 1;
}

}