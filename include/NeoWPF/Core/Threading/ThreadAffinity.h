/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
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
