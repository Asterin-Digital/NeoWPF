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
};

/*
    @summary
    Completes available input events without handling them.

    @param queue
    Queue passed to the operations.

    @param operations
    Functions used to retrieve, pre-dispatch, and complete events.

    @returns
    Number of events completed as unhandled.

    @internal
*/
inline std::size_t ADrainUnhandledInputQueue(
    void* queue, const AActivityInputQueueOperations& operations) noexcept
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
        operations.finishEvent(queue, event, 0);
        ++completed;
    }
}

}