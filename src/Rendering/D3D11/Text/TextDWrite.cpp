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
#include <array>
#include <cmath>
#include <mutex>
#include <stdexcept>

using Microsoft::WRL::ComPtr;

namespace NeoWPF::Rendering::Text {

IDWriteFontCollection* GetCustomFontCollection();

namespace {

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

        if (fileNameNoExt.size() == familyName.size()) {
            bool sameFamily = true;
            for (std::size_t i = 0; i < familyName.size(); ++i) {
                if (static_cast<wchar_t>(::towlower(static_cast<wint_t>(familyName[i]))) != static_cast<wchar_t>(::towlower(static_cast<wint_t>(fileNameNoExt[i])))) {
                    sameFamily = false;
                    break;
                }
            }
            if (sameFamily) {
                return true;
            }
        }
    }

    return false;
}

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
            ::NeoWPF::Diagnostics::LogError("GetDWriteFactory: DWriteCreateFactory failed 0x%08X", static_cast<unsigned int>(hr));
            factory = nullptr;
        }
    });
    return factory;
}

inline DWRITE_FONT_WEIGHT FontWeightToDWrite(int fontWeight) noexcept
{
    constexpr std::array<DWRITE_FONT_WEIGHT, 10> weights{
        DWRITE_FONT_WEIGHT_THIN,
        DWRITE_FONT_WEIGHT_THIN,
        DWRITE_FONT_WEIGHT_EXTRA_LIGHT,
        DWRITE_FONT_WEIGHT_LIGHT,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_WEIGHT_MEDIUM,
        DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_WEIGHT_EXTRA_BOLD,
        DWRITE_FONT_WEIGHT_BLACK
    };
    int index = fontWeight / 100;
    if (index < 0) index = 0;
    if (index >= 9) index = 9;
    return weights[index];
}

ComPtr<IDWriteTextFormat> CreateTextFormat(const std::wstring& fontFamily, int fontSize, int maxWidth, int fontWeight, NeoWPF::UI::FontStyle fontStyle)
{
    const std::wstring effectiveFontFamily = ResolveFontFamily(fontFamily);

    ComPtr<IDWriteFactory> factory = GetDWriteFactory();
    if (!factory) {
        return nullptr;
    }

    IDWriteFontCollection* customFontCollection =
        IsRegisteredCustomFontFamily(effectiveFontFamily)
            ? GetCustomFontCollection()
            : nullptr;

    DWRITE_FONT_WEIGHT dwriteWeight = FontWeightToDWrite(fontWeight);
    DWRITE_FONT_STYLE dwriteStyle = static_cast<DWRITE_FONT_STYLE>(fontStyle);

    ComPtr<IDWriteTextFormat> textFormat;
    HRESULT hr = factory->CreateTextFormat(
        effectiveFontFamily.c_str(),
        customFontCollection,
        dwriteWeight,
        dwriteStyle,
        DWRITE_FONT_STRETCH_NORMAL,
        static_cast<FLOAT>(fontSize),
        L"en-US",
        &textFormat);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("CreateTextFormat: failed for font family '%ls' with custom collection 0x%08X; falling back to system fonts", effectiveFontFamily.c_str(), static_cast<unsigned int>(hr));
        hr = factory->CreateTextFormat(
            effectiveFontFamily.c_str(),
            nullptr,
            dwriteWeight,
            dwriteStyle,
            DWRITE_FONT_STRETCH_NORMAL,
            static_cast<FLOAT>(fontSize),
            L"en-US",
            &textFormat);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogError("CreateTextFormat: system fallback for '%ls' failed 0x%08X; falling back to Segoe UI", effectiveFontFamily.c_str(), static_cast<unsigned int>(hr));
            hr = factory->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                dwriteWeight,
                dwriteStyle,
                DWRITE_FONT_STRETCH_NORMAL,
                static_cast<FLOAT>(fontSize),
                L"en-US",
                &textFormat);
            if (FAILED(hr)) {
                ::NeoWPF::Diagnostics::LogError("CreateTextFormat: fallback Segoe UI failed 0x%08X", static_cast<unsigned int>(hr));
                return nullptr;
            }
        }
    }

    if (textFormat) {
        textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        textFormat->SetWordWrapping(maxWidth > 0 ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
    }
    return textFormat;
}

}

TextMetrics TextMeasureTextInternal(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int maxWidth, int maxHeight, int fontWeight, NeoWPF::UI::FontStyle fontStyle)
{
    if (text.empty()) {
        return {};
    }

    ComPtr<IDWriteFactory> factory = GetDWriteFactory();
    if (!factory) {
        return {};
    }

    ComPtr<IDWriteTextFormat> textFormat = CreateTextFormat(fontFamily, fontSize, maxWidth, fontWeight, fontStyle);
    if (!textFormat) {
        return {};
    }

    const int effectiveMaxWidth = std::max(1, maxWidth);
    const int effectiveMaxHeight = std::max(1, maxHeight);
    const int estimatedWidth = static_cast<int>(std::max<std::size_t>(1, text.size() * static_cast<std::size_t>(fontSize)));
    float layoutWidth = maxWidth > 0 ? static_cast<float>(effectiveMaxWidth) : static_cast<float>(estimatedWidth);
    float layoutHeight = maxHeight > 0 ? static_cast<float>(effectiveMaxHeight) : static_cast<float>(std::max(1, fontSize * 2));

    ComPtr<IDWriteTextLayout> textLayout;
    HRESULT hr = factory->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.length()), textFormat.Get(), layoutWidth, layoutHeight, &textLayout);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("MeasureText: CreateTextLayout failed for font '%ls' 0x%08X", ResolveFontFamily(fontFamily).c_str(), static_cast<unsigned int>(hr));
        return {};
    }

    DWRITE_TEXT_METRICS metrics = {};
    hr = textLayout->GetMetrics(&metrics);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("MeasureText: GetMetrics failed for font '%ls' 0x%08X", ResolveFontFamily(fontFamily).c_str(), static_cast<unsigned int>(hr));
        return {};
    }

    const int resultWidth = static_cast<int>(std::lround(metrics.widthIncludingTrailingWhitespace));
    const int resultHeight = static_cast<int>(std::lround(metrics.height));
    ::NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Debug, "[TextRenderer] MeasureText: textLength=%zu fontSize=%d layout=%gx%g metrics.width=%g widthInclTrailing=%g height=%g => %dx%d",
        text.size(), fontSize, layoutWidth, layoutHeight,
        metrics.width, metrics.widthIncludingTrailingWhitespace, metrics.height,
        resultWidth, resultHeight);
    return {resultWidth, resultHeight};
}

}
