/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>

namespace NeoWPF::Rendering {

/*
    @summary
    Tracks whether one native surface generation can still be used.

    @internal
*/
class RenderSurfaceGeneration {
public:
    using UseLock = std::shared_lock<std::shared_mutex>;
    using DestructionLock = std::unique_lock<std::shared_mutex>;

    RenderSurfaceGeneration() noexcept
        : m_id(s_nextId.fetch_add(1, std::memory_order_relaxed))
    {
    }

    /*
        @summary
        Returns the unique identifier for this surface generation.

        @internal
    */
    std::uint64_t GetId() const noexcept { return m_id; }

    /*
        @summary
        Indicates whether the native surface remains usable.

        @internal
    */
    bool IsActive() const noexcept { return m_active.load(std::memory_order_acquire); }

    /*
        @summary
        Acquires shared access while the surface generation remains active.

        @internal
    */
    UseLock TryAcquireUse() const
    {
        UseLock lock(m_usageMutex);
        return IsActive() ? std::move(lock) : UseLock{};
    }

    /*
        @summary
        Acquires exclusive access for surface destruction.

        @internal
    */
    DestructionLock LockForDestruction()
    {
        DestructionLock lock(m_usageMutex);
        m_active.store(false, std::memory_order_release);
        return lock;
    }

    /*
        @summary
        Invalidates this surface generation for future rendering work.

        @internal
    */
    void Invalidate()
    {
        auto destructionLock = LockForDestruction();
    }

private:
    inline static std::atomic<std::uint64_t> s_nextId{1};
    const std::uint64_t m_id;
    mutable std::shared_mutex m_usageMutex;
    std::atomic<bool> m_active{true};
};

/*
    @summary
    Identifies the native target used by a rendering operation.

    @internal
*/
struct RenderSurface {
    void* nativeWindowHandle = nullptr;
    std::shared_ptr<RenderSurfaceGeneration> generation = std::make_shared<RenderSurfaceGeneration>();

    /*
        @summary
        Indicates whether this surface generation can still be used.

        @internal
    */
    bool IsValid() const noexcept
    {
        return nativeWindowHandle && generation && generation->IsActive();
    }

    /*
        @summary
        Acquires shared access while this surface generation remains active.

        @internal
    */
    RenderSurfaceGeneration::UseLock TryAcquireUse() const
    {
        return generation ? generation->TryAcquireUse() : RenderSurfaceGeneration::UseLock{};
    }

    /*
        @summary
        Acquires exclusive access for destruction of this surface.

        @internal
    */
    RenderSurfaceGeneration::DestructionLock LockForDestruction() const
    {
        return generation
            ? generation->LockForDestruction()
            : RenderSurfaceGeneration::DestructionLock{};
    }

    /*
        @summary
        Returns the unique identifier for this surface generation.

        @internal
    */
    std::uint64_t GetGenerationId() const noexcept
    {
        return generation ? generation->GetId() : 0;
    }

    /*
        @summary
        Invalidates this surface generation for all copied surface values.

        @internal
    */
    void Invalidate() const
    {
        if (generation) {
            generation->Invalidate();
        }
    }
};

}
