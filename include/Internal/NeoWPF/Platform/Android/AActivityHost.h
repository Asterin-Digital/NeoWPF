/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Platform/Android/AActivityLifecycle.h>

#include <android/native_activity.h>

#include <thread>

namespace NeoWPF::Platform::Android {

/*
    @summary
    Owns NativeActivity callback state and its application thread.

    @internal
*/
struct AActivityHost final {
    std::thread applicationThread;
    ANativeActivity* activity = nullptr;
    AActivityLifecycle lifecycle;
    ANativeWindowOperations windowOperations{};
    AActivityLifecycle::SurfaceAvailableCallback surfaceAvailable = nullptr;
    void* surfaceAvailableContext = nullptr;
    AActivityLifecycle::SurfaceDestroyedCallback surfaceDestroyed = nullptr;
    void* surfaceDestroyedContext = nullptr;
    AActivityLifecycle::InputQueueCallback inputQueueCreated = nullptr;
    void* inputQueueCreatedContext = nullptr;
    AActivityLifecycle::InputQueueCallback inputQueueDestroyed = nullptr;
    void* inputQueueDestroyedContext = nullptr;
    AActivityLifecycle::SurfaceDestroyedCallback activityDestroyed = nullptr;
    void* activityDestroyedContext = nullptr;
};

}