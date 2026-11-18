#include <NeoWPF/UI/Core/Media/BitmapSource.h>

namespace NeoWPF::UI::Core::Media {

BitmapSource::BitmapSource(int width, int height, std::vector<std::uint8_t> pixels)
    : m_width(width)
    , m_height(height)
    , m_pixels(std::move(pixels))
{
}

int BitmapSource::GetPixelWidth() const noexcept
{
    return m_width;
}

int BitmapSource::GetPixelHeight() const noexcept
{
    return m_height;
}

bool BitmapSource::TryGetPixels(const void*& outPixels, unsigned int& outStride) const noexcept
{
    if (m_pixels.empty() || m_width <= 0 || m_height <= 0) {
        outPixels = nullptr;
        outStride = 0;
        return false;
    }

    outPixels = m_pixels.data();
    outStride = static_cast<unsigned int>(m_width * 4);
    return true;
}

std::shared_ptr<BitmapSource> BitmapSource::Create(int width, int height, const std::vector<std::uint8_t>& pixels)
{
    if (width <= 0 || height <= 0 || pixels.size() != static_cast<size_t>(width) * static_cast<size_t>(height) * 4) {
        return nullptr;
    }
    return std::make_shared<BitmapSource>(width, height, pixels);
}

std::shared_ptr<BitmapSource> BitmapSource::Create(int width, int height, std::vector<std::uint8_t>&& pixels)
{
    if (width <= 0 || height <= 0 || pixels.size() != static_cast<size_t>(width) * static_cast<size_t>(height) * 4) {
        return nullptr;
    }
    return std::make_shared<BitmapSource>(width, height, std::move(pixels));
}

}
