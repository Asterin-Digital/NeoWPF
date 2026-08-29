#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <NeoWPF/Platform/Linux/LImageDecoding.h>

#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>

namespace NeoWPF::Platform::Linux {
namespace {

bool CalculateSize(int sourceWidth, int sourceHeight, int decodeWidth, int decodeHeight,
                  int& targetWidth, int& targetHeight)
{
    if (sourceWidth <= 0 || sourceHeight <= 0 || decodeWidth < 0 || decodeHeight < 0) {
        return false;
    }

    if (decodeWidth == 0 && decodeHeight == 0) {
        targetWidth = sourceWidth;
        targetHeight = sourceHeight;
    } else if (decodeWidth == 0) {
        targetHeight = decodeHeight;
        targetWidth = std::max(1, static_cast<int>((static_cast<long long>(sourceWidth) * targetHeight) / sourceHeight));
    } else if (decodeHeight == 0) {
        targetWidth = decodeWidth;
        targetHeight = std::max(1, static_cast<int>((static_cast<long long>(sourceHeight) * targetWidth) / sourceWidth));
    } else {
        targetWidth = decodeWidth;
        targetHeight = decodeHeight;
    }

    return targetWidth > 0 && targetHeight > 0;
}

bool CopyPixels(const unsigned char* source, int sourceWidth, int sourceHeight,
                std::vector<std::uint8_t>& outPixels, int targetWidth, int targetHeight)
{
    const std::size_t pixelCount = static_cast<std::size_t>(targetWidth) * static_cast<std::size_t>(targetHeight);
    if (pixelCount > std::numeric_limits<std::size_t>::max() / 4u) {
        return false;
    }

    std::vector<std::uint8_t> pixels(pixelCount * 4u);
    for (int y = 0; y < targetHeight; ++y) {
        const int sourceY = static_cast<int>((static_cast<long long>(y) * sourceHeight) / targetHeight);
        for (int x = 0; x < targetWidth; ++x) {
            const int sourceX = static_cast<int>((static_cast<long long>(x) * sourceWidth) / targetWidth);
            const auto* sourcePixel = source + (static_cast<std::size_t>(sourceY) * sourceWidth + sourceX) * 4u;
            auto* targetPixel = pixels.data() + (static_cast<std::size_t>(y) * targetWidth + x) * 4u;
            std::copy_n(sourcePixel, 4, targetPixel);
        }
    }

    outPixels = std::move(pixels);
    return true;
}

bool DecodeLoadedImage(unsigned char* source, int sourceWidth, int sourceHeight,
                       std::vector<std::uint8_t>& outPixels, int& outWidth, int& outHeight,
                       int decodeWidth, int decodeHeight)
{
    int targetWidth = 0;
    int targetHeight = 0;
    const bool validSize = CalculateSize(sourceWidth, sourceHeight, decodeWidth, decodeHeight,
                                         targetWidth, targetHeight);
    const bool copied = validSize && CopyPixels(source, sourceWidth, sourceHeight,
                                                 outPixels, targetWidth, targetHeight);
    stbi_image_free(source);
    if (!copied) {
        outPixels.clear();
        return false;
    }

    outWidth = targetWidth;
    outHeight = targetHeight;
    return true;
}

}

bool LImageDecoding::DecodeImageFromMemory(const std::vector<std::uint8_t>& data,
                                           std::vector<std::uint8_t>& outPixels,
                                           int& outWidth, int& outHeight,
                                           int decodeWidth, int decodeHeight) const
{
    outPixels.clear();
    if (data.empty() || data.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return false;
    }

    int sourceWidth = 0;
    int sourceHeight = 0;
    int channels = 0;
    unsigned char* source = stbi_load_from_memory(
        data.data(), static_cast<int>(data.size()), &sourceWidth, &sourceHeight, &channels, 4);
    if (!source) {
        return false;
    }

    return DecodeLoadedImage(source, sourceWidth, sourceHeight, outPixels, outWidth, outHeight,
                             decodeWidth, decodeHeight);
}

bool LImageDecoding::DecodeImageFile(const std::wstring& filePath,
                                     std::vector<std::uint8_t>& outPixels,
                                     int& outWidth, int& outHeight,
                                     int decodeWidth, int decodeHeight) const
{
    outPixels.clear();
    if (filePath.empty()) {
        return false;
    }

    const std::string pathUtf8 = NeoWPF::Platform::WideToUtf8(filePath);
    int sourceWidth = 0;
    int sourceHeight = 0;
    int channels = 0;
    unsigned char* source = stbi_load(pathUtf8.c_str(), &sourceWidth, &sourceHeight, &channels, 4);
    if (!source) {
        return false;
    }

    return DecodeLoadedImage(source, sourceWidth, sourceHeight, outPixels, outWidth, outHeight,
                             decodeWidth, decodeHeight);
}

}
