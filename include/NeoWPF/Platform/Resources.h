#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace NeoWPF::Platform {

bool LoadResourceData(const std::wstring& resourcePath, std::vector<uint8_t>& outData);

}
