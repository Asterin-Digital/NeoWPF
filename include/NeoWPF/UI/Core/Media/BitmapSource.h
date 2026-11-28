/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <NeoWPF/UI/Core/Media/ImageSource.h>

namespace NeoWPF::UI::Core::Media {

class BitmapSource : public ImageSource {
public:
    BitmapSource() = default;
    BitmapSource(int width, int height, std::vector<std::uint8_t> pixels);

    int GetPixelWidth() const noexcept override;
    int GetPixelHeight() const noexcept override;
    bool TryGetPixels(const void*& outPixels, unsigned int& outStride) const noexcept override;

    static std::shared_ptr<BitmapSource> Create(int width, int height, const std::vector<std::uint8_t>& pixels);
    static std::shared_ptr<BitmapSource> Create(int width, int height, std::vector<std::uint8_t>&& pixels);

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<std::uint8_t> m_pixels;
};

}
