#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace NeoWPF::Platform {

bool FetchHttpResource(const std::wstring& url, std::vector<uint8_t>& outData);

}
