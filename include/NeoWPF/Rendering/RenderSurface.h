/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoWPF::Rendering {

/*
    @summary
    Identifies the native target used by a rendering operation.

    @internal
*/
struct RenderSurface {
    void* nativeWindowHandle = nullptr;
};

}
