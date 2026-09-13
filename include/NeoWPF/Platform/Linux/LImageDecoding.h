/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace NeoWPF::Platform::Linux {

class LImageDecoding final {
public:
    bool DecodeImageFromMemory(const std::vector<std::uint8_t>& data,
                               std::vector<std::uint8_t>& outPixels,
                               int& outWidth, int& outHeight,
                               int decodeWidth, int decodeHeight) const;
    bool DecodeImageFile(const std::wstring& filePath,
                         std::vector<std::uint8_t>& outPixels,
                         int& outWidth, int& outHeight,
                         int decodeWidth, int decodeHeight) const;
};

}