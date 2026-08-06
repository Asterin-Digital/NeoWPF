#pragma once

#include <memory>
#include <string>
#include <vector>
#include <NeoWPF/UI/Core/Media/BitmapSource.h>

namespace NeoWPF::UI::Core::Media {

class BitmapImage : public BitmapSource {
public:
    BitmapImage() = default;
    ~BitmapImage() override = default;

    static std::shared_ptr<BitmapImage> Create(const std::wstring& uri);
    static std::shared_ptr<BitmapImage> CreateFromFile(const std::wstring& filePath);

    void BeginInit();
    void EndInit();

    void SetUriSource(const std::wstring& uri);
    const std::wstring& GetUriSource() const noexcept { return m_uriSource; }

    void SetDecodePixelWidth(int width) noexcept;
    int GetDecodePixelWidth() const noexcept { return m_decodePixelWidth; }

    void SetDecodePixelHeight(int height) noexcept;
    int GetDecodePixelHeight() const noexcept { return m_decodePixelHeight; }

    bool IsInitialized() const noexcept { return m_isInitialized; }

    int GetPixelWidth() const noexcept override;
    int GetPixelHeight() const noexcept override;
    bool TryGetPixels(const void*& outPixels, unsigned int& outStride) const noexcept override;

private:
    bool TryLoad();

    std::wstring m_uriSource;
    int m_decodePixelWidth = 0;
    int m_decodePixelHeight = 0;

    int m_width = 0;
    int m_height = 0;
    std::vector<std::uint8_t> m_pixels;
    bool m_isInitialized = false;
    bool m_isInitPending = false;
};

}
