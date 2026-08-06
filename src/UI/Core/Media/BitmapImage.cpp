#include <NeoWPF/UI/Core/Media/BitmapImage.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <objbase.h>
#include <wincodec.h>
#include <cmath>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Windowscodecs.lib")

namespace NeoWPF::UI::Core::Media {

namespace {
    bool IsLocalFileUri(const std::wstring& uri)
    {
        if (uri.empty()) {
            return false;
        }
        if (uri.rfind(L"file:///", 0) == 0 || uri.rfind(L"file://", 0) == 0) {
            return true;
        }
        if (uri.size() >= 2 && ((uri[0] >= L'A' && uri[0] <= L'Z') || (uri[0] >= L'a' && uri[0] <= L'z')) && uri[1] == L':') {
            return true;
        }
        if (uri.find(L":\\") != std::wstring::npos || uri.rfind(L"\\\\", 0) == 0) {
            return true;
        }
        return false;
    }

    std::wstring NormalizeFilePath(const std::wstring& uri)
    {
        if (uri.rfind(L"file:///", 0) == 0) {
            return uri.substr(8);
        }
        if (uri.rfind(L"file://", 0) == 0) {
            return uri.substr(7);
        }
        return uri;
    }

    bool DecodeImageFile(const std::wstring& filePath,
                         std::vector<std::uint8_t>& outPixels,
                         int& outWidth,
                         int& outHeight,
                         int decodeWidth,
                         int decodeHeight)
    {
        const HRESULT coResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        const bool comInitialized = SUCCEEDED(coResult);
        const bool shouldUninitialize = coResult == S_OK;
        if (!comInitialized) {
            NeoWPF::Diagnostics::LogError("BitmapImage::DecodeImageFile failed to initialize COM for %ls", filePath.c_str());
            return false;
        }

        bool success = false;
        IWICImagingFactory* factory = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))) {
            IWICBitmapDecoder* decoder = nullptr;
            if (SUCCEEDED(factory->CreateDecoderFromFilename(filePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder))) {
                IWICBitmapFrameDecode* frame = nullptr;
                if (SUCCEEDED(decoder->GetFrame(0, &frame))) {
                    UINT originalWidth = 0;
                    UINT originalHeight = 0;
                    if (SUCCEEDED(frame->GetSize(&originalWidth, &originalHeight))) {
                        UINT targetWidth = originalWidth;
                        UINT targetHeight = originalHeight;
                        if (decodeWidth > 0 || decodeHeight > 0) {
                            if (decodeWidth > 0 && decodeHeight > 0) {
                                targetWidth = static_cast<UINT>(decodeWidth);
                                targetHeight = static_cast<UINT>(decodeHeight);
                            } else if (decodeWidth > 0) {
                                targetWidth = static_cast<UINT>(decodeWidth);
                                targetHeight = static_cast<UINT>(std::ceil((static_cast<float>(decodeWidth) / static_cast<float>(originalWidth)) * static_cast<float>(originalHeight)));
                            } else {
                                targetHeight = static_cast<UINT>(decodeHeight);
                                targetWidth = static_cast<UINT>(std::ceil((static_cast<float>(decodeHeight) / static_cast<float>(originalHeight)) * static_cast<float>(originalWidth)));
                            }
                        }

                        IWICBitmapSource* source = frame;
                        IWICBitmapScaler* scaler = nullptr;
                        if (targetWidth != originalWidth || targetHeight != originalHeight) {
                            if (SUCCEEDED(factory->CreateBitmapScaler(&scaler))) {
                                if (SUCCEEDED(scaler->Initialize(frame, targetWidth, targetHeight, WICBitmapInterpolationModeFant))) {
                                    source = scaler;
                                } else {
                                    scaler->Release();
                                    scaler = nullptr;
                                }
                            }
                        }

                        IWICFormatConverter* converter = nullptr;
                        if (SUCCEEDED(factory->CreateFormatConverter(&converter))) {
                            if (SUCCEEDED(converter->Initialize(source, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
                                const size_t stride = static_cast<size_t>(targetWidth) * 4;
                                outPixels.assign(stride * targetHeight, 0);
                                if (SUCCEEDED(converter->CopyPixels(nullptr, static_cast<UINT>(stride), static_cast<UINT>(outPixels.size()), outPixels.data()))) {
                                    outWidth = static_cast<int>(targetWidth);
                                    outHeight = static_cast<int>(targetHeight);
                                    success = true;
                                }
                            }
                            converter->Release();
                        }

                        if (scaler) {
                            scaler->Release();
                        }
                    }
                    frame->Release();
                }
                decoder->Release();
            }
            factory->Release();
        }

        if (shouldUninitialize) {
            CoUninitialize();
        }

        return success;
    }
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

    if (!IsLocalFileUri(m_uriSource)) {
        NeoWPF::Diagnostics::LogError("BitmapImage::TryLoad only supports local files for now: %ls", m_uriSource.c_str());
        return false;
    }

    const std::wstring filePath = NormalizeFilePath(m_uriSource);
    std::vector<std::uint8_t> pixels;
    int width = 0;
    int height = 0;

    if (!DecodeImageFile(filePath, pixels, width, height, m_decodePixelWidth, m_decodePixelHeight)) {
        NeoWPF::Diagnostics::LogError("BitmapImage::TryLoad failed to decode %ls", filePath.c_str());
        return false;
    }

    m_width = width;
    m_height = height;
    m_pixels = std::move(pixels);
    return true;
}

}
