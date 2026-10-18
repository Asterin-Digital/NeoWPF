/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoWPF::Diagnostics {

/*
    @summary
    Describes diagnostics failure hook points that can be used to force init failures.

    @internal
*/
enum class FailureHookPoint {
    None = 0,
    LoadD3D11Dll,
    CreateD3D11Device,
    CreateD3D11SwapChain,
    CompileShaders,
    //DWriteCreateFactory, // dont need it for now
};

/*
    @summary
    Configures the currently enabled diagnostics failure hook point.

    @param point
    The failure hook point to enable.

    @internal
*/
void SetFailureHookPoint(FailureHookPoint point);

/*
    @summary
    Returns the currently configured diagnostics failure hook point.

    @internal
*/
FailureHookPoint GetFailureHookPoint();

}
