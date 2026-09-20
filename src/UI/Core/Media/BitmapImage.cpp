#include <NeoWPF/UI/Core/Media/BitmapImage.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <algorithm>
#include <cmath>
#include <mutex>
#include <unordered_map>

namespace NeoWPF::UI::Core::Media {

static std::unordered_map<std::string, std::shared_ptr<BitmapSource>> g_imageCache;
static std::mutex g_cacheMutex;

static bool IsLocalFileUri(const std::wstring& uri) {
    return !uri.empty() && (uri[0] == L'/' || (uri.size() > 1 && uri[1] == L':'));
}

static bool IsHttpUri(const std::wstring& uri) {
    return uri.find(L"http://") == 0 || uri.find(L"https://") == 0;
}

static bool IsResourceUri(const std::wstring& uri) {
    return uri.find(L"pack://") == 0 || uri.find(L"resource://") == 0;
}

static std::wstring NormalizeFilePath(const std::wstring& uri) {
    if (uri.find(L"file:///") == 0) {
        return uri.substr(8);
    }
    return uri;
}

static std::wstring ExtractResourcePath(const std::wstring& uri) {
    size_t pos = uri.find(L'/', 10);
    if (pos != std::wstring::npos) {
        return uri.substr(pos);
    }
    return uri;
}

std::shared_ptr<BitmapImage> BitmapImage::Create(const std::wstring& uri)
{
    auto bitmap = std::make_shared<BitmapImage>();
    bitmap->BeginInit();
    bitmap->SetUriSource(uri);
    bitmap->EndInit();
    return bitmap->IsInitialized() ? bitmap : nullptr;
}

std::shared_ptr<BitmapImage> BitmapImage::CreateFromFile(const std::wstring& filePath)
{
    return Create(filePath);
}

std::shared_ptr<BitmapImage> BitmapImage::CreateFromMemory(const std::vector<std::uint8_t>& data)
{
    auto bitmap = std::make_shared<BitmapImage>();
    bitmap->LoadFromMemory(data);
    return bitmap;
}

std::shared_ptr<BitmapImage> BitmapImage::CreateFromMemory(std::vector<std::uint8_t>&& data)
{
    auto bitmap = std::make_shared<BitmapImage>();
    bitmap->LoadFromMemory(std::move(data));
    return bitmap;
}

void BitmapImage::LoadFromMemory(const std::vector<std::uint8_t>& data)
{
    if (data.empty()) {
        NeoWPF::Diagnostics::LogError("BitmapImage::LoadFromMemory failed because input data is empty");
        return;
    }

    std::vector<std::uint8_t> pixels;
    int width = 0;
    int height = 0;

    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform || !platform->DecodeImageFromMemory(data, pixels, width, height, m_decodePixelWidth, m_decodePixelHeight)) {
        NeoWPF::Diagnostics::LogError("BitmapImage::LoadFromMemory failed to decode image data");
        return;
    }

    m_uriSource.clear();
    m_width = width;
    m_height = height;
    m_pixels = std::move(pixels);
    m_isInitialized = true;
    m_isInitPending = false;
    m_isLoading = false;
}

void BitmapImage::LoadFromMemory(std::vector<std::uint8_t>&& data)
{
    if (data.empty()) {
        NeoWPF::Diagnostics::LogError("BitmapImage::LoadFromMemory failed because input data is empty");
        return;
    }

    std::vector<std::uint8_t> pixels;
    int width = 0;
    int height = 0;

    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform || !platform->DecodeImageFromMemory(data, pixels, width, height, m_decodePixelWidth, m_decodePixelHeight)) {
        NeoWPF::Diagnostics::LogError("BitmapImage::LoadFromMemory failed to decode image data");
        return;
    }

    m_uriSource.clear();
    m_width = width;
    m_height = height;
    m_pixels = std::move(pixels);
    m_isInitialized = true;
    m_isInitPending = false;
    m_isLoading = false;
}

void BitmapImage::BeginInit()
{
    m_isInitPending = true;
}

void BitmapImage::EndInit()
{
    if (!m_isInitPending) {
        NeoWPF::Diagnostics::LogError("BitmapImage::EndInit called without BeginInit");
        return;
    }

    m_isInitPending = false;
    m_isInitialized = TryLoad();
}

void BitmapImage::SetUriSource(const std::wstring& uri)
{
    m_uriSource = uri;
    if (!m_isInitPending) {
        m_isInitialized = TryLoad();
    }
}

void BitmapImage::SetDecodePixelWidth(int width) noexcept
{
    m_decodePixelWidth = width;
    if (!m_isInitPending && !m_uriSource.empty()) {
        m_isInitialized = TryLoad();
    }
}

void BitmapImage::SetDecodePixelHeight(int height) noexcept
{
    m_decodePixelHeight = height;
    if (!m_isInitPending && !m_uriSource.empty()) {
        m_isInitialized = TryLoad();
    }
}

int BitmapImage::GetPixelWidth() const noexcept
{
    return m_width;
}

int BitmapImage::GetPixelHeight() const noexcept
{
    return m_height;
}

bool BitmapImage::TryGetPixels(const void*& outPixels, unsigned int& outStride) const noexcept
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

bool BitmapImage::TryLoad()
{
    if (m_uriSource.empty()) {
        NeoWPF::Diagnostics::LogError("BitmapImage::TryLoad failed because UriSource is empty");
        return false;
    }

    const std::string uriStr = NeoWPF::Platform::WideToUtf8(m_uriSource);
    const std::string cacheKey = uriStr + "_" +
                                std::to_string(m_decodePixelWidth) + "x" + std::to_string(m_decodePixelHeight);

    if (m_cachePolicy == CachePolicy::OnLoad) {
        std::lock_guard<std::mutex> lock(g_cacheMutex);
        auto it = g_imageCache.find(cacheKey);
        if (it != g_imageCache.end()) {
            auto source = std::dynamic_pointer_cast<BitmapSource>(it->second);
            if (source) {
                m_width = source->GetPixelWidth();
                m_height = source->GetPixelHeight();
                const void* pixels = nullptr;
                unsigned int stride = 0;
                if (source->TryGetPixels(pixels, stride) && pixels) {
                    size_t pixelSize = stride * m_height;
                    m_pixels.assign(static_cast<const uint8_t*>(pixels), 
                                   static_cast<const uint8_t*>(pixels) + pixelSize);
                    return true;
                }
            }
        }
    }

    std::vector<std::uint8_t> pixels;
    int width = 0, height = 0;
    bool success = false;

    if (IsLocalFileUri(m_uriSource)) {
        const std::wstring filePath = NormalizeFilePath(m_uriSource);
        auto* platform = NeoWPF::Platform::CurrentPlatform();
        success = platform && platform->DecodeImageFile(filePath, pixels, width, height, m_decodePixelWidth, m_decodePixelHeight);
        if (!success) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "TryLoad DecodeImageFile failed for: %ls", filePath.c_str());
        }
    } else if (IsHttpUri(m_uriSource)) {
        std::vector<uint8_t> httpData;
        auto* platform = NeoWPF::Platform::CurrentPlatform();
        if (platform && platform->FetchHttpResource(m_uriSource, httpData)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Debug, "FetchHttpResource succeeded, fetched %u bytes", static_cast<unsigned int>(httpData.size()));
            success = platform->DecodeImageFromMemory(httpData, pixels, width, height, m_decodePixelWidth, m_decodePixelHeight);
            if (!success) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "DecodeImageFromMemory failed after fetching %u bytes from: %ls", static_cast<unsigned int>(httpData.size()), m_uriSource.c_str());
            }
        } else {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "FetchHttpResource failed for: %ls", m_uriSource.c_str());
        }
    } else if (IsResourceUri(m_uriSource)) {
        std::wstring resourcePath = ExtractResourcePath(m_uriSource);
        std::vector<uint8_t> resourceData;
        auto* platform = NeoWPF::Platform::CurrentPlatform();
        if (platform && platform->LoadResourceData(resourcePath, resourceData)) {
            success = platform->DecodeImageFromMemory(resourceData, pixels, width, height, m_decodePixelWidth, m_decodePixelHeight);
            if (!success) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "DecodeImageFromMemory failed for resource: %ls", resourcePath.c_str());
            }
        } else {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "LoadResourceData failed for: %ls", resourcePath.c_str());
        }
    } else {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "Unsupported URI scheme: %ls", m_uriSource.c_str());
        return false;
    }

    if (success) {
        m_width = width;
        m_height = height;
        m_pixels = std::move(pixels);

        if (m_cachePolicy == CachePolicy::OnLoad) {
            std::lock_guard<std::mutex> lock(g_cacheMutex);
            g_imageCache[cacheKey] = std::make_shared<BitmapSource>(m_width, m_height, m_pixels);
        }
        return true;
    }

    NeoWPF::Diagnostics::LogError("BitmapImage::TryLoad failed to decode %ls", m_uriSource.c_str());
    return false;
}

void BitmapImage::BeginAsyncLoad() {
    if (m_uriSource.empty()) return;
    m_isLoading = true;
    if (m_downloadStartedCallback) m_downloadStartedCallback();

    std::weak_ptr<BitmapImage> weakSelf;
    try {
        weakSelf = shared_from_this();
    } catch (const std::bad_weak_ptr&) {
        m_isLoading = false;
        NeoWPF::Diagnostics::LogError("BitmapImage::BeginAsyncLoad requires shared ownership");
        return;
    }

    auto& appState = NeoWPF::Core::GetAppState();
    if (!appState.backgroundExecutor ||
        !appState.backgroundExecutor->Post([weakSelf]() {
            if (auto self = weakSelf.lock()) {
                self->PerformAsyncLoad();
            }
        })) {
        m_isLoading = false;
        NeoWPF::Diagnostics::LogError("BitmapImage::BeginAsyncLoad failed because the background executor is stopped");
    }
}

void BitmapImage::PerformAsyncLoad() {
    const std::string cacheKey = NeoWPF::Platform::WideToUtf8(m_uriSource) + "_" +
                                std::to_string(m_decodePixelWidth) + "x" + std::to_string(m_decodePixelHeight);

    if (m_cachePolicy != CachePolicy::None) {
        std::lock_guard<std::mutex> lock(g_cacheMutex);
        auto it = g_imageCache.find(cacheKey);
        if (it != g_imageCache.end()) {
            auto& appState = NeoWPF::Core::GetAppState();
            if (appState.mainThreadExecutor) {
                std::weak_ptr<BitmapImage> weakSelf = shared_from_this();
                appState.mainThreadExecutor->Post([weakSelf]() {
                    if (auto self = weakSelf.lock()) {
                        self->CompleteAsyncLoad();
                    }
                });
            }
            return;
        }
    }

    if (TryLoad()) {
        if (m_cachePolicy != CachePolicy::None && m_cachePolicy != CachePolicy::OnDemand) {
            std::lock_guard<std::mutex> lock(g_cacheMutex);
            g_imageCache[cacheKey] = std::make_shared<BitmapSource>(m_width, m_height, m_pixels);
        }
    }

    auto& appState = NeoWPF::Core::GetAppState();
    if (appState.mainThreadExecutor) {
        std::weak_ptr<BitmapImage> weakSelf = shared_from_this();
        appState.mainThreadExecutor->Post([weakSelf]() {
            if (auto self = weakSelf.lock()) {
                self->CompleteAsyncLoad();
            }
        });
    }
}

void BitmapImage::CompleteAsyncLoad()
{
    m_isLoading = false;
    if (m_downloadCompletedCallback) {
        m_downloadCompletedCallback();
    }
}

}
