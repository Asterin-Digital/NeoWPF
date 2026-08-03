#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <Windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <dwrite.h>
#include <wrl.h>
#include <algorithm>
#include <cmath>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace NeoWPF::Rendering::Text {

namespace {

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

    STDMETHOD(IsPixelSnappingDisabled)(void*, BOOL* isDisabled) override
    {
        *isDisabled = FALSE;
        return S_OK;
    }

    STDMETHOD(GetCurrentTransform)(void*, DWRITE_MATRIX* transform) override
    {
        transform->m11 = 1.0f; transform->m12 = 0.0f;
        transform->m21 = 0.0f; transform->m22 = 1.0f;
        transform->dx = 0.0f; transform->dy = 0.0f;
        return S_OK;
    }

    STDMETHOD(GetPixelsPerDip)(void*, FLOAT* pixelsPerDip) override
    {
        *pixelsPerDip = 1.0f;
        return S_OK;
    }

    STDMETHOD(DrawGlyphRun)(void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        const DWRITE_GLYPH_RUN* glyphRun,
        const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
        IUnknown* clientDrawingEffect) override
    {
        if (!glyphRun || !bitmap) {
            ::NeoWPF::Diagnostics::LogError("DrawGlyphRun: null glyphRun or bitmap");
            return E_INVALIDARG;
        }

        ComPtr<IDWriteRenderingParams> renderingParams;
        HRESULT hr = factory->CreateRenderingParams(&renderingParams);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogError("DrawGlyphRun: CreateRenderingParams failed 0x%08X", static_cast<unsigned int>(hr));
            return hr;
        }

        const DWRITE_RENDERING_MODE renderingMode = DWRITE_RENDERING_MODE_ALIASED;
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
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogError("DrawGlyphRun: CreateGlyphRunAnalysis failed 0x%08X", static_cast<unsigned int>(hr));
            return hr;
        }

        RECT textureBounds;
        hr = glyphRunAnalysis->GetAlphaTextureBounds(textureType, &textureBounds);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogError("DrawGlyphRun: GetAlphaTextureBounds failed 0x%08X", static_cast<unsigned int>(hr));
            return hr;
        }

        int textureWidth = textureBounds.right - textureBounds.left;
        int textureHeight = textureBounds.bottom - textureBounds.top;
        if (textureWidth <= 0 || textureHeight <= 0) {
            if (s_textRendererDebug) {
                ::NeoWPF::Diagnostics::LogInfo("DrawGlyphRun: zero texture size %dx%d", textureWidth, textureHeight);
            }
            return S_OK;
        }

        UINT32 textureBufferSize = static_cast<UINT32>(textureWidth) * static_cast<UINT32>(textureHeight);
        std::vector<BYTE> alphaData(textureBufferSize);
        hr = glyphRunAnalysis->CreateAlphaTexture(textureType, &textureBounds, alphaData.data(), textureBufferSize);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogError("DrawGlyphRun: CreateAlphaTexture failed 0x%08X", static_cast<unsigned int>(hr));
            return hr;
        }

        float adjustedBaselineY = baselineOriginY;
        float destTop = adjustedBaselineY + static_cast<float>(textureBounds.top);
        float destBottom = destTop + static_cast<float>(textureHeight);
        if (destTop < 0.0f) {
            adjustedBaselineY -= destTop;
        }
        if (destBottom > static_cast<float>(height)) {
            adjustedBaselineY -= (destBottom - static_cast<float>(height));
        }

        int originX = static_cast<int>(std::floor(baselineOriginX + textureBounds.left));
        int originY = static_cast<int>(std::floor(adjustedBaselineY + textureBounds.top));

        int extraLeft = std::max(0, -originX);
        int extraTop = std::max(0, -originY);
        int extraRight = std::max(0, originX + textureWidth - width);
        int extraBottom = std::max(0, originY + textureHeight - height);

        if (extraLeft || extraTop || extraRight || extraBottom) {
            int newWidth = width + extraLeft + extraRight;
            int newHeight = height + extraTop + extraBottom;
            std::vector<uint8_t> newBitmap(static_cast<size_t>(newWidth) * static_cast<size_t>(newHeight) * 4, 0);

            for (int r = 0; r < height; ++r) {
                for (int c = 0; c < width; ++c) {
                    size_t oldIdx = (static_cast<size_t>(r) * width + static_cast<size_t>(c)) * 4;
                    size_t newIdx = (static_cast<size_t>(r + extraTop) * newWidth + static_cast<size_t>(c + extraLeft)) * 4;
                    newBitmap[newIdx + 0] = (*bitmap)[oldIdx + 0];
                    newBitmap[newIdx + 1] = (*bitmap)[oldIdx + 1];
                    newBitmap[newIdx + 2] = (*bitmap)[oldIdx + 2];
                    newBitmap[newIdx + 3] = (*bitmap)[oldIdx + 3];
                }
            }
            (*bitmap) = std::move(newBitmap);
            originX += extraLeft;
            originY += extraTop;
            width = newWidth;
            height = newHeight;
        }

        for (int row = 0; row < textureHeight; ++row) {
            int dstY = originY + row;
            if (dstY < 0 || dstY >= height) continue;

            const BYTE* rowData = alphaData.data() + static_cast<size_t>(row) * textureWidth;
            for (int col = 0; col < textureWidth; ++col) {
                int dstX = originX + col;
                if (dstX < 0 || dstX >= width) continue;
                uint8_t alpha = rowData[col];
                if (alpha == 0) continue;
                size_t pixelIndex = (static_cast<size_t>(dstY) * width + static_cast<size_t>(dstX)) * 4;
                (*bitmap)[pixelIndex + 0] = 255;
                (*bitmap)[pixelIndex + 1] = 255;
                (*bitmap)[pixelIndex + 2] = 255;
                (*bitmap)[pixelIndex + 3] = alpha;
            }
        }
        return S_OK;
    }

    STDMETHOD(DrawUnderline)(void*, FLOAT, FLOAT, const DWRITE_UNDERLINE*, IUnknown*) override { return S_OK; }
    STDMETHOD(DrawStrikethrough)(void*, FLOAT, FLOAT, const DWRITE_STRIKETHROUGH*, IUnknown*) override { return S_OK; }
    STDMETHOD(DrawInlineObject)(void*, FLOAT, FLOAT, IDWriteInlineObject*, BOOL, BOOL, IUnknown*) override { return E_NOTIMPL; }
};

}

TextMetrics TextMeasureTextInternal(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int maxWidth, int maxHeight);

namespace {

bool CaseInsensitiveEquals(const std::wstring& lhs, const std::wstring& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (static_cast<wchar_t>(::towlower(static_cast<wint_t>(lhs[i]))) != static_cast<wchar_t>(::towlower(static_cast<wint_t>(rhs[i])))) {
            return false;
        }
    }
    return true;
}

bool IsRegisteredCustomFontFamily(const std::wstring& familyName)
{
    if (familyName.empty()) {
        return false;
    }

    for (const auto& filePath : GetRegisteredFontFiles()) {
        const std::wstring fileName = filePath.substr(filePath.find_last_of(L"\\/") + 1);
        std::wstring fileNameNoExt = fileName;
        if (const auto dot = fileNameNoExt.find_last_of(L'.'); dot != std::wstring::npos) {
            fileNameNoExt.resize(dot);
        }

        if (CaseInsensitiveEquals(fileNameNoExt, familyName)) {
            return true;
        }
    }

    return false;
}

bool TryCreateCustomFontFace(IDWriteFactory* factory, const std::wstring& familyName, ComPtr<IDWriteFontFace>& outFontFace)
{
    outFontFace = nullptr;
    if (!factory || familyName.empty()) {
        return false;
    }

    for (const auto& filePath : GetRegisteredFontFiles()) {
        const std::wstring fileName = filePath.substr(filePath.find_last_of(L"\\/") + 1);
        std::wstring fileNameNoExt = fileName;
        if (const auto dot = fileNameNoExt.find_last_of(L'.'); dot != std::wstring::npos) {
            fileNameNoExt.resize(dot);
        }

        const bool match = CaseInsensitiveEquals(fileNameNoExt, familyName) ||
            CaseInsensitiveEquals(fileNameNoExt, familyName + L".ttf") ||
            CaseInsensitiveEquals(fileNameNoExt, familyName + L".otf");
        if (!match) {
            continue;
        }

        ComPtr<IDWriteFontFile> fontFile;
        HRESULT hr = factory->CreateFontFileReference(filePath.c_str(), nullptr, &fontFile);
        if (FAILED(hr) || !fontFile) {
            continue;
        }

        BOOL isSupported = FALSE;
        DWRITE_FONT_FILE_TYPE fileType = DWRITE_FONT_FILE_TYPE_UNKNOWN;
        DWRITE_FONT_FACE_TYPE faceType = DWRITE_FONT_FACE_TYPE_UNKNOWN;
        UINT32 numberOfFaces = 0;
        hr = fontFile->Analyze(&isSupported, &fileType, &faceType, &numberOfFaces);
        if (FAILED(hr) || !isSupported || numberOfFaces == 0) {
            continue;
        }

        IDWriteFontFile* fontFileArray[1] = { fontFile.Get() };
        hr = factory->CreateFontFace(faceType, 1, fontFileArray, 0, DWRITE_FONT_SIMULATIONS_NONE, &outFontFace);
        if (SUCCEEDED(hr) && outFontFace) {
            return true;
        }
    }

    return false;
}

}

std::vector<uint8_t> TextRenderTextBitmapInternal(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int& outWidth, int& outHeight)
{
    outWidth = 0;
    outHeight = 0;
    if (text.empty()) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: empty text");
        return {};
    }

    const std::wstring effectiveFontFamily = ResolveFontFamily(fontFamily);
    if (effectiveFontFamily.empty()) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: no font family provided");
        return {};
    }

    ComPtr<IDWriteFactory> factory;
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(factory.GetAddressOf()));
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: DWriteCreateFactory failed 0x%08X", static_cast<unsigned int>(hr));
        return {};
    }

    ComPtr<IDWriteFontFace> customFontFace;
    const bool useCustomFont = IsRegisteredCustomFontFamily(effectiveFontFamily) &&
        TryCreateCustomFontFace(factory.Get(), effectiveFontFamily, customFontFace);

    if (useCustomFont && customFontFace) {
        DWRITE_FONT_METRICS metrics = {};
        customFontFace->GetMetrics(&metrics);
        const UINT16 designUnitsPerEm = metrics.designUnitsPerEm != 0 ? metrics.designUnitsPerEm : 1000;

        std::vector<UINT32> codePoints;
        codePoints.reserve(text.size());
        for (wchar_t ch : text) {
            codePoints.push_back(static_cast<UINT32>(ch));
        }

        std::vector<UINT16> glyphIndices(codePoints.size());
        if (SUCCEEDED(customFontFace->GetGlyphIndices(codePoints.data(), static_cast<UINT32>(codePoints.size()), glyphIndices.data()))) {
            std::vector<DWRITE_GLYPH_METRICS> glyphMetrics(codePoints.size());
            if (SUCCEEDED(customFontFace->GetDesignGlyphMetrics(glyphIndices.data(), static_cast<UINT32>(glyphIndices.size()), glyphMetrics.data(), FALSE))) {
                int totalAdvance = 0;
                for (const auto& metric : glyphMetrics) {
                    totalAdvance += static_cast<int>(std::lround(static_cast<double>(metric.advanceWidth) * static_cast<double>(fontSize) / static_cast<double>(designUnitsPerEm)));
                }

                outWidth = std::max(1, totalAdvance);
                outHeight = std::max(1, static_cast<int>(std::lround(static_cast<double>(metrics.ascent + metrics.descent) * static_cast<double>(fontSize) / static_cast<double>(designUnitsPerEm))));

                std::vector<FLOAT> glyphAdvances(glyphIndices.size());
                std::vector<DWRITE_GLYPH_OFFSET> glyphOffsets(glyphIndices.size());
                for (std::size_t i = 0; i < glyphIndices.size(); ++i) {
                    glyphAdvances[i] = static_cast<FLOAT>(std::lround(static_cast<double>(glyphMetrics[i].advanceWidth) * static_cast<double>(fontSize) / static_cast<double>(designUnitsPerEm)));
                }

                DWRITE_GLYPH_RUN glyphRun = {};
                glyphRun.fontFace = customFontFace.Get();
                glyphRun.fontEmSize = static_cast<FLOAT>(fontSize);
                glyphRun.glyphCount = static_cast<UINT32>(glyphIndices.size());
                glyphRun.glyphIndices = glyphIndices.data();
                glyphRun.glyphAdvances = glyphAdvances.data();
                glyphRun.glyphOffsets = glyphOffsets.data();
                glyphRun.isSideways = FALSE;
                glyphRun.bidiLevel = 0;

                std::vector<uint8_t> bitmap(static_cast<size_t>(outWidth) * static_cast<size_t>(outHeight) * 4, 0);
                BitmapTextRenderer* renderer = new BitmapTextRenderer(factory, &bitmap, outWidth, outHeight);
                hr = renderer->DrawGlyphRun(nullptr, 0.0f, static_cast<FLOAT>(fontSize), DWRITE_MEASURING_MODE_NATURAL, &glyphRun, nullptr, nullptr);
                outWidth = renderer->width;
                outHeight = renderer->height;
                renderer->Release();
                if (SUCCEEDED(hr)) {
                    return bitmap;
                }
            }
        }
    }

    IDWriteFontCollection* customFontCollection = useCustomFont ? GetCustomFontCollection() : nullptr;

    ComPtr<IDWriteTextFormat> textFormat;
    hr = factory->CreateTextFormat(
        effectiveFontFamily.c_str(),
        customFontCollection,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        static_cast<FLOAT>(fontSize),
        L"en-US",
        &textFormat);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: CreateTextFormat failed 0x%08X", static_cast<unsigned int>(hr));
        return {};
    }

    textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    ComPtr<IDWriteTextLayout> textLayout;
    hr = factory->CreateTextLayout(
        text.c_str(), static_cast<UINT32>(text.length()),
        textFormat.Get(),
        static_cast<FLOAT>(std::max(1, outWidth)),
        static_cast<FLOAT>(std::max(1, outHeight)),
        &textLayout);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: CreateTextLayout failed 0x%08X", static_cast<unsigned int>(hr));
        return {};
    }

    DWRITE_TEXT_METRICS layoutMetrics = {};
    hr = textLayout->GetMetrics(&layoutMetrics);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: GetMetrics failed 0x%08X", static_cast<unsigned int>(hr));
        return {};
    }

    outWidth = static_cast<int>(std::lround(layoutMetrics.widthIncludingTrailingWhitespace));
    outHeight = static_cast<int>(std::lround(layoutMetrics.height));
    if (outWidth <= 0 || outHeight <= 0) {
        outWidth = std::max(1, fontSize * static_cast<int>(text.length()));
        outHeight = std::max(1, fontSize + 4);
    }

    std::vector<uint8_t> bitmap(static_cast<size_t>(outWidth) * static_cast<size_t>(outHeight) * 4, 0);

    DWRITE_OVERHANG_METRICS overhang = {};
    hr = textLayout->GetOverhangMetrics(&overhang);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: GetOverhangMetrics failed 0x%08X", static_cast<unsigned int>(hr));
        return {};
    }

    UINT32 lineMetricsCount = 0;
    hr = textLayout->GetLineMetrics(nullptr, 0, &lineMetricsCount);
    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: GetLineMetrics failed preparation 0x%08X count=%u", static_cast<unsigned int>(hr), lineMetricsCount);
        return {};
    }

    std::vector<DWRITE_LINE_METRICS> lineMetrics(lineMetricsCount);
    hr = textLayout->GetLineMetrics(lineMetrics.data(), lineMetricsCount, &lineMetricsCount);
    if (FAILED(hr) || lineMetricsCount == 0) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: GetLineMetrics failed on second pass 0x%08X count=%u", static_cast<unsigned int>(hr), lineMetricsCount);
        return {};
    }

    const float baselineOriginX = overhang.left < 0.0f ? -overhang.left : 0.0f;
    const float baselineOriginY = static_cast<float>(lineMetrics[0].baseline) - std::min(0.0f, overhang.top);
    BitmapTextRenderer* renderer = new BitmapTextRenderer(factory, &bitmap, outWidth, outHeight);
    hr = textLayout->Draw(nullptr, renderer, baselineOriginX, baselineOriginY);
    outWidth = renderer->width;
    outHeight = renderer->height;
    renderer->Release();

    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: Draw failed 0x%08X", static_cast<unsigned int>(hr));
        return {};
    }

    return bitmap;
}

}
