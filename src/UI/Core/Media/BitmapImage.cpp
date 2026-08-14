#include <NeoWPF/UI/Core/Media/BitmapImage.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Core/Threading/AsyncTaskQueue.h>
#include <NeoWPF/Platform/Http.h>
#include <NeoWPF/Platform/Resources.h>
#include <NeoWPF/Platform/ImageDecoding.h>
#include <algorithm>
#include <cmath>
#include <mutex>
#include <unordered_map>
#include <windows.h>

namespace NeoWPF::UI::Core::Media {

static std::unordered_map<std::string, std::shared_ptr<BitmapSource>> g_imageCache;
static std::mutex g_cacheMutex;

static std::string ToUtf8(const std::wstring& input)
{
    if (input.empty()) {
        return {};
    }

    const int sizeNeeded = WideCharToMultiByte(
        CP_UTF8,
        0,
        input.c_str(),
        static_cast<int>(input.size()),
        nullptr,
        0,
        nullptr,
        nullptr);

    if (sizeNeeded <= 0) {
        return {};
    }

    std::string output(static_cast<size_t>(sizeNeeded), '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        input.c_str(),
        static_cast<int>(input.size()),
        output.data(),
        sizeNeeded,
        nullptr,
        nullptr);

    return output;
}

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

    const std::string uriStr = ToUtf8(m_uriSource);
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
        success = NeoWPF::Platform::DecodeImageFile(filePath, pixels, width, height, m_decodePixelWidth, m_decodePixelHeight);
        if (!success) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "TryLoad DecodeImageFile failed for: %ls", filePath.c_str());
        }
    } else if (IsHttpUri(m_uriSource)) {
        std::vector<uint8_t> httpData;
        if (NeoWPF::Platform::FetchHttpResource(m_uriSource, httpData)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Debug, "FetchHttpResource succeeded, fetched %u bytes", static_cast<unsigned int>(httpData.size()));
            success = NeoWPF::Platform::DecodeImageFromMemory(httpData, pixels, width, height, m_decodePixelWidth, m_decodePixelHeight);
            if (!success) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "DecodeImageFromMemory failed after fetching %u bytes from: %ls", static_cast<unsigned int>(httpData.size()), m_uriSource.c_str());
            }
        } else {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "FetchHttpResource failed for: %ls", m_uriSource.c_str());
        }
    } else if (IsResourceUri(m_uriSource)) {
        std::wstring resourcePath = ExtractResourcePath(m_uriSource);
        std::vector<uint8_t> resourceData;
        if (NeoWPF::Platform::LoadResourceData(resourcePath, resourceData)) {
            success = NeoWPF::Platform::DecodeImageFromMemory(resourceData, pixels, width, height, m_decodePixelWidth, m_decodePixelHeight);
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

    NeoWPF::Core::Threading::AsyncTaskQueue::GetInstance().Enqueue([this]() { PerformAsyncLoad(); });
}

void BitmapImage::PerformAsyncLoad() {
    std::shared_ptr<BitmapImage> self(this, [](void*) {});

    const std::string cacheKey = ToUtf8(m_uriSource) + "_" +
                                std::to_string(m_decodePixelWidth) + "x" + std::to_string(m_decodePixelHeight);

    if (m_cachePolicy != CachePolicy::None) {
        std::lock_guard<std::mutex> lock(g_cacheMutex);
        auto it = g_imageCache.find(cacheKey);
        if (it != g_imageCache.end()) {
            m_isLoading = false;
            if (m_downloadCompletedCallback) m_downloadCompletedCallback();
            return;
        }
    }

    if (TryLoad()) {
        if (m_cachePolicy != CachePolicy::None && m_cachePolicy != CachePolicy::OnDemand) {
            std::lock_guard<std::mutex> lock(g_cacheMutex);
            g_imageCache[cacheKey] = std::make_shared<BitmapSource>(m_width, m_height, m_pixels);
        }
    }

    m_isLoading = false;
    if (m_downloadCompletedCallback) m_downloadCompletedCallback();
}

}
