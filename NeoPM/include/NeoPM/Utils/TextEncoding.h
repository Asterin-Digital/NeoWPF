/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>

namespace NeoPM::Utils {
    std::string WideToUtf8(const std::wstring& value);
    std::wstring Utf8ToWide(const std::string& value);
}