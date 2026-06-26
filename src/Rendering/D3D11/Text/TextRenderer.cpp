#include <NeoWPF/Rendering/TextRenderer.h>
#include <Windows.h>
#include <dwrite.h>
#include <wrl.h>
#include <cmath>
#include <mutex>
#include <stdexcept>

using Microsoft::WRL::ComPtr;

namespace NeoWPF::Rendering::Text {

namespace {

ComPtr<IDWriteFactory> GetDWriteFactory()
{
    static std::once_flag factoryOnce;
    static ComPtr<IDWriteFactory> factory;
    std::call_once(factoryOnce, []() {
        HRESULT hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(factory.GetAddressOf()));
        if (FAILED(hr)) {
            throw std::runtime_error("DWriteCreateFactory failed");
        }
    });
    return factory;
}

ComPtr<IDWriteTextFormat> CreateTextFormat(int fontSize)
{
    ComPtr<IDWriteFactory> factory = GetDWriteFactory();
    ComPtr<IDWriteTextFormat> textFormat;
    HRESULT hr = factory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        static_cast<FLOAT>(fontSize),
        L"en-US",
        &textFormat);
    if (FAILED(hr)) {
        throw std::runtime_error("CreateTextFormat failed");
    }

    textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    return textFormat;
}

struct BitmapTextRenderer : public IDWriteTextRenderer {
    ComPtr<IDWriteFactory> factory;
    std::vector<uint8_t>* bitmap;
    int width;
    int height;
    long refCount = 1;

    BitmapTextRenderer(ComPtr<IDWriteFactory> factory, std::vector<uint8_t>* bitmap, int width, int height)
        : factory(std::move(factory)), bitmap(bitmap), width(width), height(height)
    {
    }

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override
    {
        if (!ppvObject) return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IDWritePixelSnapping) || riid == __uuidof(IDWriteTextRenderer)) {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() override
    {
        return static_cast<ULONG>(InterlockedIncrement(&refCount));
    }

    STDMETHOD_(ULONG, Release)() override
    {
        ULONG count = static_cast<ULONG>(InterlockedDecrement(&refCount));
        if (count == 0) {
            delete this;
        }
        return count;
    }

    STDMETHOD(IsPixelSnappingDisabled)(void* clientDrawingContext, BOOL* isDisabled) override
    {
        if (!isDisabled) return E_POINTER;
        *isDisabled = FALSE;
        return S_OK;
    }

    STDMETHOD(GetCurrentTransform)(void* clientDrawingContext, DWRITE_MATRIX* transform) override
    {
        if (!transform) return E_POINTER;
        transform->m11 = 1.0f;
        transform->m12 = 0.0f;
        transform->m21 = 0.0f;
        transform->m22 = 1.0f;
        transform->dx = 0.0f;
        transform->dy = 0.0f;
        return S_OK;
    }

    STDMETHOD(GetPixelsPerDip)(void* clientDrawingContext, FLOAT* pixelsPerDip) override
    {
        if (!pixelsPerDip) return E_POINTER;
        *pixelsPerDip = 1.0f;
        return S_OK;
    }

    STDMETHOD(DrawGlyphRun)(void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        const DWRITE_GLYPH_RUN* glyphRun,
        const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
        IUnknown* clientDrawingEffect) override
    {
        if (!glyphRun || !bitmap) return E_INVALIDARG;

        ComPtr<IDWriteRenderingParams> renderingParams;
        HRESULT hr = factory->CreateRenderingParams(&renderingParams);
        if (FAILED(hr)) return hr;

        const DWRITE_RENDERING_MODE renderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC;
        const DWRITE_TEXTURE_TYPE textureType = DWRITE_TEXTURE_ALIASED_1x1;
        ComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
        hr = factory->CreateGlyphRunAnalysis(
            glyphRun,
            1.0f,
            nullptr,
            renderingMode,
            measuringMode,
            baselineOriginX,
            baselineOriginY,
            &glyphRunAnalysis);
        if (FAILED(hr)) return hr;

        RECT textureBounds;
        hr = glyphRunAnalysis->GetAlphaTextureBounds(textureType, &textureBounds);
        if (FAILED(hr)) return hr;

        int textureWidth = textureBounds.right - textureBounds.left;
        int textureHeight = textureBounds.bottom - textureBounds.top;
        if (textureWidth <= 0 || textureHeight <= 0) return S_OK;

        UINT32 textureBufferSize = static_cast<UINT32>(textureWidth) * static_cast<UINT32>(textureHeight);
        std::vector<BYTE> alphaData(textureBufferSize);
        hr = glyphRunAnalysis->CreateAlphaTexture(textureType,
            &textureBounds,
            alphaData.data(),
            textureBufferSize);
        if (FAILED(hr)) return hr;

        int originX = textureBounds.left;
        int originY = textureBounds.top;

        for (int row = 0; row < textureHeight; ++row) {
            int dstY = originY + row;
            if (dstY < 0 || dstY >= height) continue;

            const BYTE* rowData = alphaData.data() + static_cast<size_t>(row) * textureWidth;
            for (int col = 0; col < textureWidth; ++col) {
                int dstX = originX + col;
                if (dstX < 0 || dstX >= width) continue;
                uint8_t alpha = rowData[col];
                size_t pixelIndex = (static_cast<size_t>(dstY) * width + static_cast<size_t>(dstX)) * 4;
                (*bitmap)[pixelIndex + 0] = 255;
                (*bitmap)[pixelIndex + 1] = 255;
                (*bitmap)[pixelIndex + 2] = 255;
                (*bitmap)[pixelIndex + 3] = alpha;
            }
        }

        return S_OK;
    }

    STDMETHOD(DrawUnderline)(void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY,
        const DWRITE_UNDERLINE* underline, IUnknown* clientDrawingEffect) override
    {
        return S_OK;
    }

    STDMETHOD(DrawStrikethrough)(void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY,
        const DWRITE_STRIKETHROUGH* strikethrough, IUnknown* clientDrawingEffect) override
    {
        return S_OK;
    }

    STDMETHOD(DrawInlineObject)(void* clientDrawingContext, FLOAT originX, FLOAT originY,
        IDWriteInlineObject* inlineObject, BOOL isSideways, BOOL isRightToLeft,
        IUnknown* clientDrawingEffect) override
    {
        return E_NOTIMPL;
    }
};

}

TextMetrics MeasureText(const std::wstring& text, int fontSize, int maxWidth, int maxHeight)
{
    if (text.empty()) {
        return {};
    }

    ComPtr<IDWriteFactory> factory = GetDWriteFactory();
    ComPtr<IDWriteTextFormat> textFormat = CreateTextFormat(fontSize);

    float layoutWidth = maxWidth > 0 ? static_cast<float>(maxWidth) : static_cast<float>(text.size() * fontSize);
    float layoutHeight = maxHeight > 0 ? static_cast<float>(maxHeight) : static_cast<float>(fontSize * 2);

    ComPtr<IDWriteTextLayout> textLayout;
    HRESULT hr = factory->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.length()), textFormat.Get(), layoutWidth, layoutHeight, &textLayout);
    if (FAILED(hr)) {
        return {};
    }

    DWRITE_TEXT_METRICS metrics = {};
    hr = textLayout->GetMetrics(&metrics);
    if (FAILED(hr)) {
        return {};
    }

    return {
        static_cast<int>(std::ceil(metrics.widthIncludingTrailingWhitespace)),
        static_cast<int>(std::ceil(metrics.height))
    };
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int& outWidth, int& outHeight)
{
    outWidth = 0;
    outHeight = 0;
    if (text.empty()) {
        return {};
    }

    TextMetrics metrics = MeasureText(text, fontSize, 0, 0);
    if (metrics.width <= 0 || metrics.height <= 0) {
        return {};
    }

    outWidth = metrics.width;
    outHeight = metrics.height;
    std::vector<uint8_t> bitmap(static_cast<size_t>(outWidth) * static_cast<size_t>(outHeight) * 4);

    ComPtr<IDWriteFactory> factory = GetDWriteFactory();
    ComPtr<IDWriteTextFormat> textFormat = CreateTextFormat(fontSize);

    ComPtr<IDWriteTextLayout> textLayout;
    HRESULT hr = factory->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.length()), textFormat.Get(), static_cast<FLOAT>(outWidth), static_cast<FLOAT>(outHeight), &textLayout);
    if (FAILED(hr)) {
        return {};
    }

    BitmapTextRenderer* renderer = new BitmapTextRenderer(factory, &bitmap, outWidth, outHeight);
    hr = textLayout->Draw(nullptr, renderer, 0.0f, 0.0f);
    renderer->Release();

    if (FAILED(hr)) {
        return {};
    }

    return bitmap;
}

}
