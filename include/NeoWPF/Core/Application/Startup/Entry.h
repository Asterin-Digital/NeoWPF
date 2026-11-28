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

#include <NeoWPF/Core/Application/LaunchContext.h>

namespace NeoWPF::Core {

/*
    @summary
    Runs the application using the supplied launch information.

    @param launch
    Read-only information supplied by the platform entry point.

    @returns
    The application exit code.
*/
int Entry(const LaunchContext& launch);

}
