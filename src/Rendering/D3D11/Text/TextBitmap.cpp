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
#include <climits>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace NeoWPF::Rendering::Text {

namespace {

DWRITE_RENDERING_MODE SelectRenderingMode(float fontSize)
{

    return DWRITE_RENDERING_MODE_NATURAL;
}

struct BitmapTextRenderer : public IDWriteTextRenderer {
    ComPtr<IDWriteFactory> factory;
    std::vector<uint8_t>* bitmap;
    int width;
    int height;
    int inkLeft = INT_MAX;
    int inkTop = INT_MAX;
    int inkRight = INT_MIN;
    int inkBottom = INT_MIN;
    long refCount = 1;

    BitmapTextRenderer(ComPtr<IDWriteFactory> factory, std::vector<uint8_t>* bitmap, int width, int height)
        : factory(std::move(factory)), bitmap(bitmap), width(width), height(height)
    {
    }

    bool GetInkBounds(int& outLeft, int& outTop, int& outWidth, int& outHeight) const
    {
        if (inkLeft == INT_MAX || inkTop == INT_MAX || inkRight == INT_MIN || inkBottom == INT_MIN) {
            return false;
        }
        outLeft = inkLeft;
        outTop = inkTop;
        outWidth = inkRight - inkLeft;
        outHeight = inkBottom - inkTop;
        return outWidth > 0 && outHeight > 0;
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
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: ENTER baseline=(%.2f, %.2f) glyphCount=%u bitmap=%p width=%d height=%d",
        baselineOriginX, baselineOriginY, glyphRun ? glyphRun->glyphCount : 0, bitmap, width, height);
    
    if (!glyphRun || !bitmap) {
        ::NeoWPF::Diagnostics::LogError("DrawGlyphRun: null glyphRun or bitmap");
        return E_INVALIDARG;
    }

    const DWRITE_RENDERING_MODE renderingMode = SelectRenderingMode(glyphRun->fontEmSize);

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: renderingMode selected: %d", static_cast<int>(renderingMode));

    const DWRITE_TEXTURE_TYPE textureType =
        (renderingMode == DWRITE_RENDERING_MODE_NATURAL ||
         renderingMode == DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC ||
         renderingMode == DWRITE_RENDERING_MODE_GDI_NATURAL ||
         renderingMode == DWRITE_RENDERING_MODE_GDI_CLASSIC)
        ? DWRITE_TEXTURE_CLEARTYPE_3x1
        : DWRITE_TEXTURE_ALIASED_1x1;

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
        "DrawGlyphRun: fontEmSize=%.2f baseline=(%.2f, %.2f) glyphCount=%u measuringMode=%d renderingMode=%d textureType=%d",
        glyphRun->fontEmSize, baselineOriginX, baselineOriginY, glyphRun->glyphCount,
        measuringMode, static_cast<int>(renderingMode), static_cast<int>(textureType));

    ComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: calling CreateGlyphRunAnalysis with origin=(0,0), caller baseline=(%.2f, %.2f)", baselineOriginX, baselineOriginY);
    
    HRESULT hr = factory->CreateGlyphRunAnalysis(
        glyphRun,
        1.0f,
        nullptr,
        renderingMode,
        measuringMode,
        0.0f,
        0.0f,
        &glyphRunAnalysis);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("DrawGlyphRun: CreateGlyphRunAnalysis failed 0x%08X", static_cast<unsigned int>(hr));
        return hr;
    }
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: CreateGlyphRunAnalysis succeeded");

    RECT textureBounds;
    hr = glyphRunAnalysis->GetAlphaTextureBounds(textureType, &textureBounds);
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: GetAlphaTextureBounds returned hr=0x%08X", static_cast<unsigned int>(hr));
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
        "DrawGlyphRun: textureBounds={l=%ld,t=%ld,r=%ld,b=%ld} width=%d height=%d",
        static_cast<long>(textureBounds.left), static_cast<long>(textureBounds.top),
        static_cast<long>(textureBounds.right), static_cast<long>(textureBounds.bottom),
        textureBounds.right - textureBounds.left, textureBounds.bottom - textureBounds.top);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("DrawGlyphRun: GetAlphaTextureBounds failed 0x%08X", static_cast<unsigned int>(hr));
        return hr;
    }

    int textureWidth = textureBounds.right - textureBounds.left;
    int textureHeight = textureBounds.bottom - textureBounds.top;
    if (textureWidth <= 0 || textureHeight <= 0) {
        ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
            "DrawGlyphRun: zero texture size %dx%d; skipping bitmap generation", textureWidth, textureHeight);
        return S_OK;
    }

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: valid texture size %dx%d, proceeding to allocate alpha buffer", textureWidth, textureHeight);

    const size_t bytesPerPixel = (textureType == DWRITE_TEXTURE_CLEARTYPE_3x1) ? 3 : 1;
    const UINT32 textureBufferSize = static_cast<UINT32>(textureWidth * textureHeight * bytesPerPixel);
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: allocating buffer: bytesPerPixel=%zu totalBytes=%u", bytesPerPixel, textureBufferSize);
    
    std::vector<BYTE> alphaData(textureBufferSize);
    hr = glyphRunAnalysis->CreateAlphaTexture(textureType, &textureBounds, alphaData.data(), textureBufferSize);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("DrawGlyphRun: CreateAlphaTexture failed 0x%08X", static_cast<unsigned int>(hr));
        return hr;
    }
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: CreateAlphaTexture succeeded, buffer size=%zu bytes", alphaData.size());

    int originX = static_cast<int>(std::lround(baselineOriginX + static_cast<float>(textureBounds.left)));
    int originY = static_cast<int>(std::lround(baselineOriginY + static_cast<float>(textureBounds.top)));

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: texture bounds in (0,0) space, adding baseline offset: origin=(%d,%d)  bitmapSize=%dx%d", 
        originX, originY, width, height);

    // Expand bitmap if texture extends beyond current bounds
    int extraLeft = std::max(0, -originX);
    int extraTop = std::max(0, -originY);
    int extraRight = std::max(0, originX + textureWidth - width);
    int extraBottom = std::max(0, originY + textureHeight - height);

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
        "DrawGlyphRun: expand needed: left=%d top=%d right=%d bottom=%d", 
        extraLeft, extraTop, extraRight, extraBottom);

    if (extraLeft || extraTop || extraRight || extraBottom) {
        int newWidth = width + extraLeft + extraRight;
        int newHeight = height + extraTop + extraBottom;
        ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
            "DrawGlyphRun: expanding bitmap from %dx%d to %dx%d", width, height, newWidth, newHeight);
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
        ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
            "DrawGlyphRun: after expand: origin=(%d,%d) bitmapSize=%dx%d", 
            originX, originY, width, height);
    }

    if (originX < inkLeft) inkLeft = originX;
    if (originY < inkTop) inkTop = originY;
    inkRight = std::max(inkRight, originX + textureWidth);
    inkBottom = std::max(inkBottom, originY + textureHeight);

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
        "DrawGlyphRun: ink bounds updated: left=%d top=%d right=%d bottom=%d", 
        inkLeft, inkTop, inkRight, inkBottom);

    int pixelsDrawn = 0;
    for (int row = 0; row < textureHeight; ++row) {
        int dstY = originY + row;
        if (dstY < 0 || dstY >= height) continue;

        const BYTE* rowData = alphaData.data() + static_cast<size_t>(row) * textureWidth * bytesPerPixel;
        for (int col = 0; col < textureWidth; ++col) {
            int dstX = originX + col;
            if (dstX < 0 || dstX >= width) continue;

            uint8_t alpha;
            if (textureType == DWRITE_TEXTURE_CLEARTYPE_3x1) {

                alpha = (rowData[col * 3] + rowData[col * 3 + 1] + rowData[col * 3 + 2]) / 3;
            } else {
                alpha = rowData[col];
            }

            if (alpha == 0) continue;
            size_t pixelIndex = (static_cast<size_t>(dstY) * width + static_cast<size_t>(dstX)) * 4;

            (*bitmap)[pixelIndex + 0] = 255;
            (*bitmap)[pixelIndex + 1] = 255;
            (*bitmap)[pixelIndex + 2] = 255;
            (*bitmap)[pixelIndex + 3] = alpha;
            pixelsDrawn++;
        }
    }
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
        "DrawGlyphRun: finished rendering, pixels drawn=%d", pixelsDrawn);

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "DrawGlyphRun: EXIT with S_OK");
    
    return S_OK;
}

    STDMETHOD(DrawUnderline)(void*, FLOAT, FLOAT, const DWRITE_UNDERLINE*, IUnknown*) override { return S_OK; }
    STDMETHOD(DrawStrikethrough)(void*, FLOAT, FLOAT, const DWRITE_STRIKETHROUGH*, IUnknown*) override { return S_OK; }
    STDMETHOD(DrawInlineObject)(void*, FLOAT, FLOAT, IDWriteInlineObject*, BOOL, BOOL, IUnknown*) override { return E_NOTIMPL; }
};

}

TextMetrics TextMeasureTextInternal(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int maxWidth, int maxHeight, int fontWeight, NeoWPF::UI::FontStyle fontStyle);
std::vector<uint8_t> TextRenderTextBitmapInternal(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int fontWeight, NeoWPF::UI::FontStyle fontStyle, int& outWidth, int& outHeight, int* outInkTop = nullptr, int* outInkHeight = nullptr);

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

std::vector<uint8_t> TextRenderTextBitmapInternal(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int fontWeight, NeoWPF::UI::FontStyle fontStyle, int& outWidth, int& outHeight, int* outInkTop /*= nullptr*/, int* outInkHeight /*= nullptr*/)
{
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Info,
        "TextRenderTextBitmapInternal: ENTER text='%ls' fontSize=%d maxWidth=%d maxHeight=%d fontFamily='%ls'", 
        text.c_str(), fontSize, maxWidth, maxHeight, fontFamily.c_str());
    
    outWidth = 0;
    outHeight = 0;
    if (outInkTop) {
        *outInkTop = 0;
    }
    if (outInkHeight) {
        *outInkHeight = 0;
    }
    
    if (text.empty()) {
        ::NeoWPF::Diagnostics::LogError("TextRenderTextBitmapInternal: empty text");
        return {};
    }

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "TextRenderTextBitmapInternal: resolving font family");
    
    const std::wstring effectiveFontFamily = ResolveFontFamily(fontFamily);
    if (effectiveFontFamily.empty()) {
        ::NeoWPF::Diagnostics::LogError("TextRenderTextBitmapInternal: no font family provided");
        return {};
    }
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "TextRenderTextBitmapInternal: resolved font family to '%ls'", effectiveFontFamily.c_str());

    ComPtr<IDWriteFactory> factory;
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(factory.GetAddressOf()));
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("TextRenderTextBitmapInternal: DWriteCreateFactory failed 0x%08X", static_cast<unsigned int>(hr));
        return {};
    }

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Trace,
        "TextRenderTextBitmapInternal: DWrite factory created");

    ComPtr<IDWriteFontFace> customFontFace;
    const bool useCustomFont = IsRegisteredCustomFontFamily(effectiveFontFamily) &&
        TryCreateCustomFontFace(factory.Get(), effectiveFontFamily, customFontFace);

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
        "TextRenderTextBitmapInternal: useCustomFont=%d", useCustomFont);

    if (useCustomFont && customFontFace) {
        ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Info,
            "TextRenderTextBitmapInternal: using CUSTOM font");
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

                const float baselineOriginY = static_cast<float>(
                    std::lround(static_cast<double>(metrics.ascent) * static_cast<double>(fontSize) / static_cast<double>(designUnitsPerEm)));

                ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
                    "TextRenderTextBitmapInternal (CUSTOM): outWidth=%d outHeight=%d baselineOriginY=%.2f metrics.ascent=%u fontSize=%d designUnitsPerEm=%u", 
                    outWidth, outHeight, baselineOriginY, metrics.ascent, fontSize, designUnitsPerEm);

                std::vector<uint8_t> bitmap(static_cast<size_t>(outWidth) * static_cast<size_t>(outHeight) * 4, 0);
                BitmapTextRenderer* renderer = new BitmapTextRenderer(factory, &bitmap, outWidth, outHeight);
                hr = renderer->DrawGlyphRun(nullptr, 0.0f, baselineOriginY, DWRITE_MEASURING_MODE_NATURAL, &glyphRun, nullptr, nullptr);
                outWidth = renderer->width;
                outHeight = renderer->height;
                
                ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
                    "TextRenderTextBitmapInternal (CUSTOM): after DrawGlyphRun: renderer.width=%d renderer.height=%d", 
                    renderer->width, renderer->height);
                
    if (outInkTop || outInkHeight) {
        int inkLeft = 0, inkTop = 0, inkWidth = 0, inkHeight = 0;
        if (renderer->GetInkBounds(inkLeft, inkTop, inkWidth, inkHeight)) {
            ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
                "TextRenderTextBitmapInternal (CUSTOM): ink bounds from renderer: left=%d top=%d width=%d height=%d", 
                inkLeft, inkTop, inkWidth, inkHeight);
            if (outInkHeight) *outInkHeight = inkHeight;
        } else {
            if (outInkHeight) *outInkHeight = outHeight;
        }
        // inkTop is always the bitmap top, which is -baselineOriginY from baseline
        int baselineAsInt = static_cast<int>(std::lround(baselineOriginY));
        if (outInkTop) *outInkTop = -baselineAsInt;
        ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
            "TextRenderTextBitmapInternal (CUSTOM): reporting inkTop=%d (bitmap top, always -baseline=%d)", -baselineAsInt, baselineAsInt);
    }
                renderer->Release();
                if (SUCCEEDED(hr)) {
                    return bitmap;
                }
            }
        }
    }

    ComPtr<IDWriteFontCollection> systemFontCollection;
    hr = factory->GetSystemFontCollection(&systemFontCollection);
    if (FAILED(hr) || !systemFontCollection) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: GetSystemFontCollection failed 0x%08X", static_cast<unsigned int>(hr));
        return {};
    }

    UINT32 familyIndex = 0;
    BOOL exists = FALSE;
    systemFontCollection->FindFamilyName(effectiveFontFamily.c_str(), &familyIndex, &exists);
    if (!exists) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: font family '%ls' not found in system collection", effectiveFontFamily.c_str());
        return {};
    }

    ComPtr<IDWriteFontFamily> matchedFamily;
    systemFontCollection->GetFontFamily(familyIndex, &matchedFamily);
    if (!matchedFamily) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: failed to get matched font family for '%ls'", effectiveFontFamily.c_str());
        return {};
    }

    ComPtr<IDWriteFont> matchedFont;

    // Convert fontWeight to DWRITE_FONT_WEIGHT
    DWRITE_FONT_WEIGHT dwriteWeight = DWRITE_FONT_WEIGHT_NORMAL;
    if (fontWeight >= 900) {
        dwriteWeight = DWRITE_FONT_WEIGHT_BLACK;
    } else if (fontWeight >= 800) {
        dwriteWeight = DWRITE_FONT_WEIGHT_EXTRA_BOLD;
    } else if (fontWeight >= 700) {
        dwriteWeight = DWRITE_FONT_WEIGHT_BOLD;
    } else if (fontWeight >= 600) {
        dwriteWeight = DWRITE_FONT_WEIGHT_SEMI_BOLD;
    } else if (fontWeight >= 500) {
        dwriteWeight = DWRITE_FONT_WEIGHT_MEDIUM;
    } else if (fontWeight >= 400) {
        dwriteWeight = DWRITE_FONT_WEIGHT_NORMAL;
    } else if (fontWeight >= 300) {
        dwriteWeight = DWRITE_FONT_WEIGHT_LIGHT;
    } else if (fontWeight >= 200) {
        dwriteWeight = DWRITE_FONT_WEIGHT_EXTRA_LIGHT;
    } else {
        dwriteWeight = DWRITE_FONT_WEIGHT_THIN;
    }

    DWRITE_FONT_STYLE dwriteStyle = static_cast<DWRITE_FONT_STYLE>(fontStyle);

    matchedFamily->GetFirstMatchingFont(
        dwriteWeight,
        DWRITE_FONT_STRETCH_NORMAL,
        dwriteStyle,
        &matchedFont);
    if (!matchedFont) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: failed to get first matching font for '%ls'", effectiveFontFamily.c_str());
        return {};
    }

    ComPtr<IDWriteFontFace> systemFontFace;
    matchedFont->CreateFontFace(&systemFontFace);
    if (!systemFontFace) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: failed to create system font face for '%ls'", effectiveFontFamily.c_str());
        return {};
    }

    DWRITE_FONT_METRICS metrics = {};
    systemFontFace->GetMetrics(&metrics);
    const UINT16 designUnitsPerEm = metrics.designUnitsPerEm != 0 ? metrics.designUnitsPerEm : 1000;

    std::vector<UINT32> codePoints;
    codePoints.reserve(text.size());
    for (wchar_t ch : text) {
        codePoints.push_back(static_cast<UINT32>(ch));
    }

    std::vector<UINT16> glyphIndices(codePoints.size());
    if (FAILED(systemFontFace->GetGlyphIndices(codePoints.data(), static_cast<UINT32>(codePoints.size()), glyphIndices.data()))) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: GetGlyphIndices failed for '%ls'", effectiveFontFamily.c_str());
        return {};
    }

    std::vector<DWRITE_GLYPH_METRICS> glyphMetrics(codePoints.size());
    if (FAILED(systemFontFace->GetDesignGlyphMetrics(glyphIndices.data(), static_cast<UINT32>(glyphIndices.size()), glyphMetrics.data(), FALSE))) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: GetDesignGlyphMetrics failed for '%ls'", effectiveFontFamily.c_str());
        return {};
    }

    int totalAdvance = 0;
    for (const auto& metric : glyphMetrics) {
        totalAdvance += static_cast<int>(std::lround(static_cast<double>(metric.advanceWidth) * static_cast<double>(fontSize) / static_cast<double>(designUnitsPerEm)));
    }

    outWidth = std::max(1, totalAdvance);
    const int ascent = static_cast<int>(std::lround(static_cast<double>(metrics.ascent) * static_cast<double>(fontSize) / static_cast<double>(designUnitsPerEm)));
    const int descent = static_cast<int>(std::lround(static_cast<double>(metrics.descent) * static_cast<double>(fontSize) / static_cast<double>(designUnitsPerEm)));
    outHeight = std::max(1, ascent + std::max(0, descent));

    std::vector<FLOAT> glyphAdvances(glyphIndices.size());
    std::vector<DWRITE_GLYPH_OFFSET> glyphOffsets(glyphIndices.size(), { 0.0f, 0.0f });
    for (std::size_t i = 0; i < glyphIndices.size(); ++i) {
        glyphAdvances[i] = static_cast<FLOAT>(std::lround(static_cast<double>(glyphMetrics[i].advanceWidth) * static_cast<double>(fontSize) / static_cast<double>(designUnitsPerEm)));
    }

    DWRITE_GLYPH_RUN glyphRun = {};
    glyphRun.fontFace = systemFontFace.Get();
    glyphRun.fontEmSize = static_cast<FLOAT>(fontSize);
    glyphRun.glyphCount = static_cast<UINT32>(glyphIndices.size());
    glyphRun.glyphIndices = glyphIndices.data();
    glyphRun.glyphAdvances = glyphAdvances.data();
    glyphRun.glyphOffsets = glyphOffsets.data();
    glyphRun.isSideways = FALSE;
    glyphRun.bidiLevel = 0;

    std::vector<uint8_t> bitmap(static_cast<size_t>(outWidth) * static_cast<size_t>(outHeight) * 4, 0);
    BitmapTextRenderer* renderer = new BitmapTextRenderer(factory, &bitmap, outWidth, outHeight);
    const float baselineOriginY = static_cast<float>(ascent);
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
        "TextRenderTextBitmapInternal (SYSTEM): outWidth=%d outHeight=%d baselineOriginY=%.2f ascent=%d descent=%d fontSize=%d designUnitsPerEm=%u", 
        outWidth, outHeight, baselineOriginY, ascent, descent, fontSize, designUnitsPerEm);
    
    hr = renderer->DrawGlyphRun(nullptr, 0.0f, baselineOriginY, DWRITE_MEASURING_MODE_NATURAL, &glyphRun, nullptr, nullptr);
    outWidth = renderer->width;
    outHeight = renderer->height;
    
    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
        "TextRenderTextBitmapInternal (SYSTEM): after DrawGlyphRun: renderer.width=%d renderer.height=%d", 
        renderer->width, renderer->height);
    
    if (outInkTop || outInkHeight) {
        int inkLeft = 0, inkTop = 0, inkWidth = 0, inkHeight = 0;
        if (renderer->GetInkBounds(inkLeft, inkTop, inkWidth, inkHeight)) {
            ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
                "TextRenderTextBitmapInternal (SYSTEM): ink bounds from renderer: left=%d top=%d width=%d height=%d", 
                inkLeft, inkTop, inkWidth, inkHeight);
            if (outInkHeight) *outInkHeight = inkHeight;
        } else {
            if (outInkHeight) *outInkHeight = outHeight;
        }
        // inkTop is always the bitmap top, which is -ascent from baseline
        if (outInkTop) *outInkTop = -ascent;
        ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::TextRendering, ::NeoWPF::Diagnostics::Level::Debug,
            "TextRenderTextBitmapInternal (SYSTEM): reporting inkTop=%d (bitmap top, always -ascent=%d)", -ascent, ascent);
    }
    renderer->Release();

    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("RenderTextBitmap: DrawGlyphRun failed 0x%08X", static_cast<unsigned int>(hr));
        return {};
    }

    return bitmap;
}

}
