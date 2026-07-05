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
#include <mutex>
#include <stdexcept>

using Microsoft::WRL::ComPtr;

namespace NeoWPF::Rendering::Text {

namespace {

static bool s_textRendererDebug = false;

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

}

TextMetrics TextMeasureTextInternal(const std::wstring& text, int fontSize, int maxWidth, int maxHeight)
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

    DWRITE_OVERHANG_METRICS overhang = {};
    hr = textLayout->GetOverhangMetrics(&overhang);
    if (FAILED(hr)) {
        return {};
    }

    float desiredWidth = metrics.widthIncludingTrailingWhitespace;
    if (overhang.left < 0.0f) {
        desiredWidth += -overhang.left;
    }
    if (overhang.right > 0.0f) {
        desiredWidth += overhang.right;
    }

    float desiredHeight = metrics.height;
    if (overhang.top < 0.0f) {
        desiredHeight += -overhang.top;
    }
    if (overhang.bottom > 0.0f) {
        desiredHeight += overhang.bottom;
    }

    int resultWidth = static_cast<int>(std::ceil(desiredWidth));
    int resultHeight = static_cast<int>(std::ceil(desiredHeight));
    if (s_textRendererDebug) {
        ::NeoWPF::Diagnostics::LogInfo("[TextRenderer] MeasureText: textLength=%zu fontSize=%d layout=%gx%g metrics.width=%g widthInclTrailing=%g overhang.l=%g overhang.r=%g => %dx%d",
            text.size(), fontSize, layoutWidth, layoutHeight,
            metrics.width, metrics.widthIncludingTrailingWhitespace,
            overhang.left, overhang.right,
            resultWidth, resultHeight);
    }
    return {resultWidth, resultHeight};
}

}
