/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowing.h>

#include <atomic>
#include <cstdint>
#include <functional>

namespace NeoWPF::Core {

/*
    @summary
    Tracks whether a presentation host is active or permanently shut down.

    @internal
*/
class ApplicationHostLifecycle {
public:
    enum class State : std::uint8_t {
        Inactive,
        Active,
        Shutdown,
    };

    /*
        @summary
        Activates the host unless it has been shut down.

        @returns
        True when the host is active after the operation.

        @internal
    */
    bool Activate() noexcept
    {
        State state = m_state.load(std::memory_order_acquire);
        while (state != State::Shutdown) {
            if (state == State::Active ||
                m_state.compare_exchange_weak(
                    state,
                    State::Active,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire)) {
                return true;
            }
        }
        return false;
    }

    /*
        @summary
        Deactivates the host without shutting it down.

        @internal
    */
    void Deactivate() noexcept
    {
        State active = State::Active;
        m_state.compare_exchange_strong(
            active,
            State::Inactive,
            std::memory_order_acq_rel,
            std::memory_order_acquire);
    }

    /*
        @summary
        Permanently shuts down the host.

        @internal
    */
    void Shutdown() noexcept { m_state.store(State::Shutdown, std::memory_order_release); }

    /*
        @summary
        Indicates whether the host is currently active.

        @internal
    */
    bool IsActive() const noexcept { return m_state.load(std::memory_order_acquire) == State::Active; }

    /*
        @summary
        Indicates whether the host has been permanently shut down.

        @internal
    */
    bool IsShutdown() const noexcept { return m_state.load(std::memory_order_acquire) == State::Shutdown; }

private:
    std::atomic<State> m_state{State::Inactive};
};

/*
    @summary
    Defines host-controlled message processing for the application runtime.

    @internal
*/
class IApplicationHost {
public:
    using FrameCallback = std::function<bool()>;

    virtual ~IApplicationHost() = default;

    /*
        @summary
        Runs the host message loop and calls the frame callback between pumps.

        @param frameCallback
        Returns true to continue the loop.

        @returns
        The host exit code.

        @internal
    */
    virtual int Run(const FrameCallback& frameCallback) = 0;

    /*
        @summary
        Requests that the host loop exit with the supplied code.

        @param exitCode
        The code returned by the host loop.

        @internal
    */
    virtual void RequestQuit(int exitCode) noexcept = 0;

    /*
        @summary
        Permanently shuts down the host independently of a loop exit request.

        @internal
    */
    virtual void Shutdown() noexcept = 0;

    /*
        @summary
        Indicates whether the host is currently active.

        @internal
    */
    virtual bool IsActive() const noexcept = 0;

    /*
        @summary
        Indicates whether the host has been permanently shut down.

        @internal
    */
    virtual bool IsShutdown() const noexcept = 0;
};

/*
    @summary
    Adapts desktop window messages to application frame processing.

    @internal
*/
class DesktopApplicationHost final : public IApplicationHost {
public:
    explicit DesktopApplicationHost(NeoWPF::Contracts::IWindowing& windowing) noexcept;

    /*
        @summary
        Pumps desktop messages and runs frames until either requests exit.

        @internal
    */
    int Run(const FrameCallback& frameCallback) override;

    /*
        @summary
        Forwards an exit request to desktop windowing.

        @internal
    */
    void RequestQuit(int exitCode) noexcept override;

    /*
        @summary
        Permanently shuts down the desktop host.

        @internal
    */
    void Shutdown() noexcept override;

    /*
        @summary
        Indicates whether the desktop host is currently active.

        @internal
    */
    bool IsActive() const noexcept override;

    /*
        @summary
        Indicates whether the desktop host has been permanently shut down.

        @internal
    */
    bool IsShutdown() const noexcept override;

private:
    NeoWPF::Contracts::IWindowing& m_windowing;
    ApplicationHostLifecycle m_lifecycle;
};

}
