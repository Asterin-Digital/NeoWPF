/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowing.h>

#include <functional>

namespace NeoWPF::Core {

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

private:
    NeoWPF::Contracts::IWindowing& m_windowing;
};

}
