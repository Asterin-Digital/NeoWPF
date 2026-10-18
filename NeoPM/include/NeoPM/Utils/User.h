/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <optional>
#include <string>

namespace NeoPM::Utils {

class User {
public:
    static std::optional<std::wstring> GetCurrentUsername();
};

}
