/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <NeoWPF/UI/Core/Media/BitmapSource.h>

namespace NeoWPF::UI::Core::Media {

enum class CachePolicy {
    None,
    OnLoad,
    OnDemand
};

class BitmapImage : public BitmapSource, public std::enable_shared_from_this<BitmapImage> {
public:
    using DownloadStartedCallback = std::function<void()>;
    using DownloadProgressCallback = std::function<void(int bytesRead, int totalBytes)>;
    using DownloadCompletedCallback = std::function<void()>;

    BitmapImage() = default;
    ~BitmapImage() override = default;

    static std::shared_ptr<BitmapImage> Create(const std::wstring& uri);
    static std::shared_ptr<BitmapImage> CreateFromFile(const std::wstring& filePath);
    static std::shared_ptr<BitmapImage> CreateFromMemory(const std::vector<std::uint8_t>& data);
    static std::shared_ptr<BitmapImage> CreateFromMemory(std::vector<std::uint8_t>&& data);

    void LoadFromMemory(const std::vector<std::uint8_t>& data);
    void LoadFromMemory(std::vector<std::uint8_t>&& data);

    void BeginInit();
    void EndInit();

    void SetUriSource(const std::wstring& uri);
    const std::wstring& GetUriSource() const noexcept { return m_uriSource; }

    void SetDecodePixelWidth(int width) noexcept;
    int GetDecodePixelWidth() const noexcept { return m_decodePixelWidth; }

    void SetDecodePixelHeight(int height) noexcept;
    int GetDecodePixelHeight() const noexcept { return m_decodePixelHeight; }

    void SetCachePolicy(CachePolicy policy) noexcept { m_cachePolicy = policy; }
    CachePolicy GetCachePolicy() const noexcept { return m_cachePolicy; }

    bool IsInitialized() const noexcept { return m_isInitialized; }
    bool IsLoading() const noexcept { return m_isLoading; }

    void SetDownloadStartedCallback(DownloadStartedCallback callback) noexcept {
        m_downloadStartedCallback = callback;
    }

    void SetDownloadProgressCallback(DownloadProgressCallback callback) noexcept {
        m_downloadProgressCallback = callback;
    }

    void SetDownloadCompletedCallback(DownloadCompletedCallback callback) noexcept {
        m_downloadCompletedCallback = callback;
    }

    void BeginAsyncLoad();

    int GetPixelWidth() const noexcept override;
    int GetPixelHeight() const noexcept override;
    bool TryGetPixels(const void*& outPixels, unsigned int& outStride) const noexcept override;

private:
    bool TryLoad();
    void PerformAsyncLoad();
    void CompleteAsyncLoad();

    std::wstring m_uriSource;
    int m_decodePixelWidth = 0;
    int m_decodePixelHeight = 0;
    CachePolicy m_cachePolicy = CachePolicy::OnLoad;

    int m_width = 0;
    int m_height = 0;
    std::vector<std::uint8_t> m_pixels;
    bool m_isInitialized = false;
    bool m_isInitPending = false;
    bool m_isLoading = false;

    DownloadStartedCallback m_downloadStartedCallback;
    DownloadProgressCallback m_downloadProgressCallback;
    DownloadCompletedCallback m_downloadCompletedCallback;
};

}
