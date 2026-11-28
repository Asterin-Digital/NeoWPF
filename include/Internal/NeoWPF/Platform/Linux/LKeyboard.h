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

#include <cstdint>

namespace NeoWPF::Platform::Linux {

/*
    @summary
    Converts an XKB keysym to a NeoWPF key code.

    @param keysym
    XKB keysym to translate.

    @returns
    The matching key code, or zero when it is not mapped.

    @internal
*/
std::uint32_t TranslateXkbKeysymToVirtualKey(std::uint32_t keysym) noexcept;

}
