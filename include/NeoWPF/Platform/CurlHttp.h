/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace NeoWPF::Platform {

class CurlHttp final {
public:
    bool Fetch(const std::wstring& url, std::vector<std::uint8_t>& outData) const;
};

}