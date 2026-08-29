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