/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowing.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

namespace NeoWPF::UI {
class Window;
}

namespace NeoWPF::Core {

/*
    @summary
    Owns application-host lifecycle state and the stable main content host.

    @internal
*/
class ApplicationHostSession {
public:
    enum class State : std::uint8_t {
        Inactive,
        Active,
        Shutdown,
    };

    explicit ApplicationHostSession(std::shared_ptr<NeoWPF::UI::Window> contentHost = {}) noexcept
        : m_contentHost(std::move(contentHost))
    {
    }

    /*
        @summary
        Activates the host session unless it has been shut down.

        @returns
        True when the host session is active after the operation.

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
        Deactivates the host session without shutting it down.

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
        Permanently shuts down the host session.

        @internal
    */
    void Shutdown() noexcept { m_state.store(State::Shutdown, std::memory_order_release); }

    /*
        @summary
        Indicates whether the host session is currently active.

        @internal
    */
    bool IsActive() const noexcept { return m_state.load(std::memory_order_acquire) == State::Active; }

    /*
        @summary
        Indicates whether the host session has been permanently shut down.

        @internal
    */
    bool IsShutdown() const noexcept { return m_state.load(std::memory_order_acquire) == State::Shutdown; }

    /*
        @summary
        Resets lifecycle state for a new application run.

        @internal
    */
    void Begin() noexcept { m_state.store(State::Inactive, std::memory_order_release); }

    /*
        @summary
        Replaces the stable main content host.

        @param contentHost
        The new main content host.

        @internal
    */
    void SetContentHost(std::shared_ptr<NeoWPF::UI::Window> contentHost) noexcept
    {
        m_contentHost = std::move(contentHost);
    }

    /*
        @summary
        Returns the stable main content host.

        @internal
    */
    const std::shared_ptr<NeoWPF::UI::Window>& GetContentHost() const noexcept { return m_contentHost; }

private:
    std::atomic<State> m_state{State::Inactive};
    std::shared_ptr<NeoWPF::UI::Window> m_contentHost;
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
    DesktopApplicationHost(
        NeoWPF::Contracts::IWindowing& windowing,
        ApplicationHostSession& session) noexcept;

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
    ApplicationHostSession& m_session;
};

}
