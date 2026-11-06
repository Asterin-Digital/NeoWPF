/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>

namespace NeoWPF::Platform::Android {

/*
    @summary
    Provides operations for draining an Android input queue.

    @internal
*/
struct AActivityInputQueueOperations {
    int (*getEvent)(void*, void**) noexcept = nullptr;
    int (*preDispatchEvent)(void*, void*) noexcept = nullptr;
    void (*finishEvent)(void*, void*, int) noexcept = nullptr;
    int (*processEvent)(void*, void*) noexcept = nullptr;
};

/*
    @summary
    Processes and completes available input events.

    @param queue
    Queue passed to the operations.

    @param context
    Context passed to the event-processing callback.

    @param operations
    Functions used to retrieve, pre-dispatch, and complete events.

    @returns
    Number of events completed.

    @internal
*/
inline std::size_t ADrainInputQueue(
    void* queue, void* context,
    const AActivityInputQueueOperations& operations) noexcept
{
    if (!queue || !operations.getEvent || !operations.preDispatchEvent ||
        !operations.finishEvent) {
        return 0;
    }

    std::size_t completed = 0;
    for (;;) {
        void* event = nullptr;
        if (operations.getEvent(queue, &event) < 0 || !event) {
            return completed;
        }
        if (operations.preDispatchEvent(queue, event) != 0) {
            continue;
        }
        const int handled = operations.processEvent
            ? operations.processEvent(context, event)
            : 0;
        operations.finishEvent(queue, event, handled);
        ++completed;
    }
}

}