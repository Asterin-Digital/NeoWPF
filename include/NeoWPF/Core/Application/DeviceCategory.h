/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>

namespace NeoWPF::Core {

/*
    @summary
    Identifies the device category hosting the application.
*/
enum class DeviceCategory : std::uint8_t {
    Desktop,
    Mobile,
};

}