#include <NeoWPF/Platform/Android/AWindowing.h>
#include <NeoWPF/Platform/Android/AActivityInputQueue.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <unistd.h>

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
    const int result = AInputQueue_preDispatchEvent(
        static_cast<AInputQueue*>(queue), inputEvent);
    if (result != 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "Android key event pre-dispatched by system: keyCode=%d action=%d",
            AKeyEvent_getKeyCode(inputEvent), AKeyEvent_getAction(inputEvent));
    }
    return result;
}

void FinishInputQueueEvent(void* queue, void* event, int handled) noexcept
{
    AInputQueue_finishEvent(static_cast<AInputQueue*>(queue),
                            static_cast<AInputEvent*>(event), handled);
}

}

int AWindowing::HandleInputQueueEvent(void* event) noexcept
{
    auto* const inputEvent = static_cast<AInputEvent*>(event);
    const int eventType = AInputEvent_getType(inputEvent);
    if (eventType == AINPUT_EVENT_TYPE_KEY) {
        return HandleKeyInputEvent(inputEvent);
    }
    if (eventType == AINPUT_EVENT_TYPE_MOTION) {
        return HandlePointerInputEvent(inputEvent);
    }
    if (eventType != AINPUT_EVENT_TYPE_KEY) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "Android input event not handled: unsupported type=%d", eventType);
    }
    return 0;
}

void AWindowing::ProcessInputQueueEvent(Event& event) noexcept
{
    if (event.type == EventType::InputQueueCreated) {
        if (!event.inputQueue || !m_looper || m_inputQueue == event.inputQueue) {
            const char* const reason = !event.inputQueue ? "null queue" :
                (!m_looper ? "null looper" : "already attached");
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
                NeoWPF::Diagnostics::Level::Debug,
                "T3 input queue attach skipped queue=%p looper=%p current=%p reason=%s tid=%d",
                event.inputQueue, m_looper, m_inputQueue, reason, gettid());
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
                NeoWPF::Diagnostics::Level::Debug,
                "Android input queue attach skipped: queue=%p looper=%p currentQueue=%p",
                event.inputQueue, m_looper, m_inputQueue);
            return;
        }
        if (m_inputQueue) {
            AInputQueue_detachLooper(m_inputQueue);
        }
        AInputQueue_attachLooper(event.inputQueue, m_looper, 0,
                                 &AWindowing::InputQueueLooperCallback, this);
        m_inputQueue = event.inputQueue;
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "T3 input queue attached queue=%p looper=%p tid=%d",
            m_inputQueue, m_looper, gettid());
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "Android input queue attached: queue=%p looper=%p",
            m_inputQueue, m_looper);
        return;
    }

    if (event.type == EventType::InputQueueDestroyed && event.inputQueue &&
        m_inputQueue == event.inputQueue) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "Android input queue detached: queue=%p", m_inputQueue);
        AInputQueue_detachLooper(m_inputQueue);
        m_inputQueue = nullptr;
    }
}

void AWindowing::StopInputQueue() noexcept
{
    if (!m_inputQueue) {
        return;
    }
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
        NeoWPF::Diagnostics::Level::Debug,
        "Android input queue stopped: queue=%p", m_inputQueue);
    AInputQueue_detachLooper(m_inputQueue);
    m_inputQueue = nullptr;
}

int AWindowing::InputQueueLooperCallback(int, int, void* context) noexcept
{
    auto* const windowing = static_cast<AWindowing*>(context);
    if (windowing && windowing->m_inputQueue) {
        const AActivityInputQueueOperations operations{
            &GetInputQueueEvent,
            &PreDispatchInputQueueEvent,
            &FinishInputQueueEvent,
            [](void* eventContext, void* event) noexcept {
                return static_cast<AWindowing*>(eventContext)->HandleInputQueueEvent(event);
            },
        };
        ADrainInputQueue(windowing->m_inputQueue, windowing, operations);
    }
    return 1;
}

}