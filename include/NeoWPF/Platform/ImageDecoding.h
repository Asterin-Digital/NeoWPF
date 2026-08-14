#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace NeoWPF::Platform {

bool DecodeImageFromMemory(const std::vector<uint8_t>& data, std::vector<uint8_t>& outPixels,
                           int& outWidth, int& outHeight, int decodeWidth, int decodeHeight);

bool DecodeImageFile(const std::wstring& filePath, std::vector<uint8_t>& outPixels,
                     int& outWidth, int& outHeight, int decodeWidth, int decodeHeight);

}
