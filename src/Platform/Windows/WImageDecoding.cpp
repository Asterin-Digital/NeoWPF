#include <NeoWPF/Diagnostics/Logger.h>
#include <objbase.h>
#include <wincodec.h>
#include <cmath>
#include <vector>
#include <string>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Windowscodecs.lib")

namespace NeoWPF::Platform {

bool DecodeImageFromMemory(const std::vector<uint8_t>& data, std::vector<uint8_t>& outPixels,
                           int& outWidth, int& outHeight, int decodeWidth, int decodeHeight) {
    if (data.empty()) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: Input data is empty");
        return false;
    }

    const HRESULT coResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool shouldUninitialize = SUCCEEDED(coResult);
    if (!SUCCEEDED(coResult)) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: CoInitializeEx failed, HRESULT: 0x%08X", coResult);
        return false;
    }

    bool success = false;
    IWICImagingFactory* factory = nullptr;
    if (!SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: CoCreateInstance for WICImagingFactory failed");
        if (shouldUninitialize) CoUninitialize();
        return false;
    }

    IWICStream* stream = nullptr;
    if (!SUCCEEDED(factory->CreateStream(&stream))) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: CreateStream failed");
        factory->Release();
        if (shouldUninitialize) CoUninitialize();
        return false;
    }

    if (!SUCCEEDED(stream->InitializeFromMemory(const_cast<BYTE*>(data.data()), (DWORD)data.size()))) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: InitializeFromMemory failed for %u bytes", (unsigned int)data.size());
        stream->Release();
        factory->Release();
        if (shouldUninitialize) CoUninitialize();
        return false;
    }

    IWICBitmapDecoder* decoder = nullptr;
    if (!SUCCEEDED(factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder))) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: CreateDecoderFromStream failed");
        stream->Release();
        factory->Release();
        if (shouldUninitialize) CoUninitialize();
        return false;
    }

    IWICBitmapFrameDecode* frame = nullptr;
    if (!SUCCEEDED(decoder->GetFrame(0, &frame))) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: GetFrame failed");
        decoder->Release();
        stream->Release();
        factory->Release();
        if (shouldUninitialize) CoUninitialize();
        return false;
    }

    UINT originalWidth = 0, originalHeight = 0;
    if (!SUCCEEDED(frame->GetSize(&originalWidth, &originalHeight))) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: GetSize failed");
        frame->Release();
        decoder->Release();
        stream->Release();
        factory->Release();
        if (shouldUninitialize) CoUninitialize();
        return false;
    }

    UINT targetWidth = originalWidth, targetHeight = originalHeight;
    if (decodeWidth > 0 || decodeHeight > 0) {
        if (decodeWidth > 0 && decodeHeight > 0) {
            targetWidth = decodeWidth;
            targetHeight = decodeHeight;
        } else if (decodeWidth > 0) {
            targetWidth = decodeWidth;
            targetHeight = (UINT)std::ceil((float)decodeWidth / originalWidth * originalHeight);
        } else {
            targetHeight = decodeHeight;
            targetWidth = (UINT)std::ceil((float)decodeHeight / originalHeight * originalWidth);
        }
    }

    IWICBitmapSource* source = frame;
    IWICBitmapScaler* scaler = nullptr;
    if (targetWidth != originalWidth || targetHeight != originalHeight) {
        if (!SUCCEEDED(factory->CreateBitmapScaler(&scaler))) {
            NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: CreateBitmapScaler failed");
            frame->Release();
            decoder->Release();
            stream->Release();
            factory->Release();
            if (shouldUninitialize) CoUninitialize();
            return false;
        }
        if (!SUCCEEDED(scaler->Initialize(frame, targetWidth, targetHeight, WICBitmapInterpolationModeFant))) {
            NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: Scaler Initialize failed");
            scaler->Release();
            frame->Release();
            decoder->Release();
            stream->Release();
            factory->Release();
            if (shouldUninitialize) CoUninitialize();
            return false;
        }
        source = scaler;
    }

    IWICFormatConverter* converter = nullptr;
    if (!SUCCEEDED(factory->CreateFormatConverter(&converter))) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: CreateFormatConverter failed");
        if (scaler) scaler->Release();
        frame->Release();
        decoder->Release();
        stream->Release();
        factory->Release();
        if (shouldUninitialize) CoUninitialize();
        return false;
    }

    if (!SUCCEEDED(converter->Initialize(source, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: Converter Initialize failed");
        converter->Release();
        if (scaler) scaler->Release();
        frame->Release();
        decoder->Release();
        stream->Release();
        factory->Release();
        if (shouldUninitialize) CoUninitialize();
        return false;
    }

    size_t stride = targetWidth * 4;
    outPixels.assign(stride * targetHeight, 0);
    if (!SUCCEEDED(converter->CopyPixels(nullptr, (UINT)stride, (UINT)outPixels.size(), outPixels.data()))) {
        NeoWPF::Diagnostics::LogError("DecodeImageFromMemory: CopyPixels failed");
        converter->Release();
        if (scaler) scaler->Release();
        frame->Release();
        decoder->Release();
        stream->Release();
        factory->Release();
        if (shouldUninitialize) CoUninitialize();
        return false;
    }

    outWidth = targetWidth;
    outHeight = targetHeight;
    success = true;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Image, NeoWPF::Diagnostics::Level::Info, "Successfully decoded %ux%u image", targetWidth, targetHeight);

    converter->Release();
    if (scaler) scaler->Release();
    frame->Release();
    decoder->Release();
    stream->Release();
    factory->Release();

    if (shouldUninitialize) CoUninitialize();
    return success;
}

bool DecodeImageFile(const std::wstring& filePath, std::vector<uint8_t>& outPixels,
                     int& outWidth, int& outHeight, int decodeWidth, int decodeHeight) {
    const HRESULT coResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool shouldUninitialize = SUCCEEDED(coResult);
    if (!SUCCEEDED(coResult)) return false;

    bool success = false;
    IWICImagingFactory* factory = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))) {
        IWICBitmapDecoder* decoder = nullptr;
        if (SUCCEEDED(factory->CreateDecoderFromFilename(filePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder))) {
            IWICBitmapFrameDecode* frame = nullptr;
            if (SUCCEEDED(decoder->GetFrame(0, &frame))) {
                UINT originalWidth = 0, originalHeight = 0;
                if (SUCCEEDED(frame->GetSize(&originalWidth, &originalHeight))) {
                    UINT targetWidth = originalWidth, targetHeight = originalHeight;
                    if (decodeWidth > 0 || decodeHeight > 0) {
                        if (decodeWidth > 0 && decodeHeight > 0) {
                            targetWidth = decodeWidth;
                            targetHeight = decodeHeight;
                        } else if (decodeWidth > 0) {
                            targetWidth = decodeWidth;
                            targetHeight = (UINT)std::ceil((float)decodeWidth / originalWidth * originalHeight);
                        } else {
                            targetHeight = decodeHeight;
                            targetWidth = (UINT)std::ceil((float)decodeHeight / originalHeight * originalWidth);
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
                            size_t stride = targetWidth * 4;
                            outPixels.assign(stride * targetHeight, 0);
                            if (SUCCEEDED(converter->CopyPixels(nullptr, (UINT)stride, (UINT)outPixels.size(), outPixels.data()))) {
                                outWidth = targetWidth;
                                outHeight = targetHeight;
                                success = true;
                            }
                        }
                        converter->Release();
                    }
                    if (scaler) scaler->Release();
                }
                frame->Release();
            }
            decoder->Release();
        }
        factory->Release();
    }

    if (shouldUninitialize) CoUninitialize();
    return success;
}

}
