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

namespace NeoWPF::UI {
class Window;
}

namespace NeoWPF::Core {

/*
    @summary
    Stores lifecycle state shared by copies of one application or host session.

    @internal
*/
class SessionLifecycle {
public:
    enum class State : std::uint8_t {
        Inactive,
        Active,
        Shutdown,
    };

    /*
        @summary
        Activates the session unless it has been shut down.

        @returns
        True when the session is active after the operation.

        @internal
    */
    bool Activate() noexcept;

    /*
        @summary
        Deactivates the session without shutting it down.

        @internal
    */
    void Deactivate() noexcept;

    /*
        @summary
        Permanently shuts down the session.

        @internal
    */
    void Shutdown() noexcept;

    /*
        @summary
        Resets the session for a new application run.

        @internal
    */
    void Begin() noexcept;

    /*
        @summary
        Indicates whether the session is active.

        @internal
    */
    bool IsActive() const noexcept;

    /*
        @summary
        Indicates whether the session has been shut down.

        @internal
    */
    bool IsShutdown() const noexcept;

private:
    struct SharedState {
        std::atomic<State> state{State::Inactive};
    };

    std::shared_ptr<SharedState> m_sharedState = std::make_shared<SharedState>();
};

/*
    @summary
    Owns application lifecycle state and the stable main content host.

    @internal
*/
class ApplicationSession {
public:
    explicit ApplicationSession(std::shared_ptr<NeoWPF::UI::Window> contentHost = {}) noexcept;

    /*
        @summary
        Activates the application session unless it has been shut down.

        @returns
        True when the application session is active after the operation.

        @internal
    */
    bool Activate() noexcept;

    /*
        @summary
        Deactivates the application session without shutting it down.

        @internal
    */
    void Deactivate() noexcept;

    /*
        @summary
        Permanently shuts down the application session.

        @internal
    */
    void Shutdown() noexcept;

    /*
        @summary
        Indicates whether the application session is currently active.

        @internal
    */
    bool IsActive() const noexcept;

    /*
        @summary
        Indicates whether the application session has been permanently shut down.

        @internal
    */
    bool IsShutdown() const noexcept;

    /*
        @summary
        Resets lifecycle state for a new application run.

        @internal
    */
    void Begin() noexcept;

    /*
        @summary
        Replaces the stable main content host.

        @param contentHost
        The new main content host.

        @internal
    */
    void SetContentHost(std::shared_ptr<NeoWPF::UI::Window> contentHost) noexcept;

    /*
        @summary
        Returns the stable main content host.

        @internal
    */
    const std::shared_ptr<NeoWPF::UI::Window>& GetContentHost() const noexcept;

    /*
        @summary
        Selects the content host for a desktop window.

        @param explicitHost
        The content host explicitly supplied by the caller.

        @param isPrimary
        True when the window will become the primary application window.

        @returns
        The explicit host, the configured main host for the primary window, or an empty pointer.

        @internal
    */
    std::shared_ptr<NeoWPF::UI::Window> SelectContentHost(
        const std::shared_ptr<NeoWPF::UI::Window>& explicitHost,
        bool isPrimary) const noexcept;

private:
    SessionLifecycle m_lifecycle;
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
        ApplicationSession& session) noexcept;

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
    ApplicationSession& m_session;
};

}
