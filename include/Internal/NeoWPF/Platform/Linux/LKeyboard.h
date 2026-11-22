/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
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
