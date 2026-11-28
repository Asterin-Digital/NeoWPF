/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <thread>

namespace NeoWPF::Core::Threading {

enum class ThreadKind {
    Unknown,
    Main,
    Render,
};

using ThreadId = std::thread::id;

class ThreadAffinity {
public:
    static ThreadId GetCurrentThreadId() noexcept;
    static ThreadKind GetCurrentThreadKind() noexcept;
    static void RegisterMainThread() noexcept;
    static void RegisterRenderThread() noexcept;
    static void ClearMainThread() noexcept;
    static void ClearRenderThread() noexcept;
    static bool IsMainThread() noexcept;
    static bool IsMainThreadRegistered() noexcept;
    static bool IsRenderThread() noexcept;
};

}
