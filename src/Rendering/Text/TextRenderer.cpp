#include <NeoWPF/Rendering/TextRenderer.h>

#include <NeoWPF/Rendering/Text/FontDatabase.h>
#include <NeoWPF/Rendering/Text/TextRasterizer.h>
#include <NeoWPF/Rendering/Text/TextShaper.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <algorithm>
#include <filesystem>
#include <mutex>

namespace NeoWPF::Rendering::Text {
namespace {

using NeoWPF::Platform::Utf8ToWide;
using NeoWPF::Platform::WideToUtf8;

FontRequest MakeRequest(const std::wstring& family, int weight, NeoWPF::UI::FontStyle style)
{
    return {WideToUtf8(family), weight > 0 ? weight : 400, style};
}

TextBitmap Rasterize(const std::wstring& text, int fontSize, const std::wstring& family,
                    int weight, NeoWPF::UI::FontStyle style)
{
    auto& fonts = GetFontDatabase();
    const auto request = MakeRequest(family, weight, style);
    const auto glyphs = CreateHarfBuzzTextShaper(fonts)->Shape(WideToUtf8(text), request, fontSize);
    auto bitmap = CreateFreeTypeTextRasterizer(fonts)->Rasterize(glyphs, request, fontSize);
    std::size_t nonzeroAlphaBytes = 0;
    for (std::size_t index = 3; index < bitmap.rgba.size(); index += 4) {
        nonzeroAlphaBytes += bitmap.rgba[index] != 0 ? 1 : 0;
    }
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Debug,
        "[TextRenderer] textLength=%zu family='%ls' fontSize=%d glyphs=%zu bitmap=%dx%d stride=%d rgbaBytes=%zu nonzeroAlpha=%zu",
        text.size(), family.c_str(), fontSize, glyphs.size(), bitmap.width, bitmap.height,
        bitmap.stride, bitmap.rgba.size(), nonzeroAlphaBytes);
    return bitmap;
}

TextMetrics Measure(const std::wstring& text, int fontSize, const std::wstring& family,
                    int maxWidth, int maxHeight, int weight, NeoWPF::UI::FontStyle style)
{
    const auto bitmap = Rasterize(text, fontSize, family, weight, style);
    TextMetrics metrics;
    metrics.width = bitmap.width;
    metrics.height = bitmap.height;
    metrics.lineHeight = bitmap.height;
    metrics.baseline = bitmap.inkTop < 0 ? -bitmap.inkTop : bitmap.inkTop;
    if (maxWidth > 0) {
        metrics.width = (std::min)(metrics.width, maxWidth);
    }
    if (maxHeight > 0) {
        metrics.height = (std::min)(metrics.height, maxHeight);
    }
    return metrics;
}

std::vector<std::wstring>& RegisteredFiles()
{
    static std::vector<std::wstring> files;
    return files;
}

std::mutex& RegisteredFilesMutex()
{
    static std::mutex mutex;
    return mutex;
}

}

std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily)
{
    const auto& records = GetFontDatabase().Records();
    if (records.empty()) {
        return requestedFontFamily;
    }
    if (!requestedFontFamily.empty()) {
        const auto requested = WideToUtf8(requestedFontFamily);
        for (const auto& record : records) {
            if (record && record->familyUtf8 == requested) {
                return requestedFontFamily;
            }
        }
    }
    return Utf8ToWide(records.front()->familyUtf8);
}

bool RegisterFontFile(const std::wstring& filePath)
{
    const bool registered = GetFontDatabase().RegisterFile(std::filesystem::path(filePath));
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Info,
        "[TextRenderer] RegisterFontFile path='%ls' result=%d records=%zu",
        filePath.c_str(), registered ? 1 : 0, GetFontDatabase().Records().size());
    if (!registered) {
        return false;
    }
    std::lock_guard<std::mutex> lock(RegisteredFilesMutex());
    RegisteredFiles().push_back(filePath);
    return true;
}

bool RegisterFontMemory(const unsigned char* data, size_t size, const std::wstring& familyName)
{
    if (!data || size == 0) {
        return false;
    }
    auto bytes = std::make_shared<std::vector<std::byte>>(size);
    std::transform(data, data + size, bytes->begin(), [](unsigned char value) {
        return static_cast<std::byte>(value);
    });
    const bool registered = GetFontDatabase().RegisterMemory(std::move(bytes), WideToUtf8(familyName));
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Info,
        "[TextRenderer] RegisterFontMemory family='%ls' bytes=%zu result=%d records=%zu",
        familyName.c_str(), size, registered ? 1 : 0, GetFontDatabase().Records().size());
    return registered;
}

std::vector<std::wstring> GetRegisteredFontFiles()
{
    std::lock_guard<std::mutex> lock(RegisteredFilesMutex());
    return RegisteredFiles();
}

TextMetrics MeasureText(const std::wstring& text, int fontSize, int maxWidth, int maxHeight)
{
    return MeasureText(text, fontSize, {}, maxWidth, maxHeight, 400, NeoWPF::UI::FontStyle::Normal);
}

TextMetrics MeasureText(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                        int maxWidth, int maxHeight)
{
    return MeasureText(text, fontSize, fontFamily, maxWidth, maxHeight, 400, NeoWPF::UI::FontStyle::Normal);
}

TextMetrics MeasureText(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                        int maxWidth, int maxHeight, int fontWeight, NeoWPF::UI::FontStyle fontStyle)
{
    return Measure(text, fontSize, fontFamily, maxWidth, maxHeight, fontWeight, fontStyle);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize,
                                      int& outWidth, int& outHeight)
{
    return RenderTextBitmap(text, fontSize, 0, 0, {}, 400, NeoWPF::UI::FontStyle::Normal,
                            outWidth, outHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth,
                                      int maxHeight, int& outWidth, int& outHeight)
{
    return RenderTextBitmap(text, fontSize, maxWidth, maxHeight, {}, 400,
                            NeoWPF::UI::FontStyle::Normal, outWidth, outHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize,
                                      const std::wstring& fontFamily, int& outWidth, int& outHeight)
{
    return RenderTextBitmap(text, fontSize, 0, 0, fontFamily, 400,
                            NeoWPF::UI::FontStyle::Normal, outWidth, outHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth,
                                      int maxHeight, const std::wstring& fontFamily,
                                      int& outWidth, int& outHeight)
{
    return RenderTextBitmap(text, fontSize, maxWidth, maxHeight, fontFamily, 400,
                            NeoWPF::UI::FontStyle::Normal, outWidth, outHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth,
                                      int maxHeight, const std::wstring& fontFamily,
                                      int& outWidth, int& outHeight, int& outInkTop,
                                      int& outInkHeight)
{
    return RenderTextBitmap(text, fontSize, maxWidth, maxHeight, fontFamily, 400,
                            NeoWPF::UI::FontStyle::Normal, outWidth, outHeight,
                            outInkTop, outInkHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth,
                                      int maxHeight, const std::wstring& fontFamily, int fontWeight,
                                      NeoWPF::UI::FontStyle fontStyle, int& outWidth,
                                      int& outHeight)
{
    int inkTop = 0;
    int inkHeight = 0;
    return RenderTextBitmap(text, fontSize, maxWidth, maxHeight, fontFamily, fontWeight,
                            fontStyle, outWidth, outHeight, inkTop, inkHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth,
                                      int maxHeight, const std::wstring& fontFamily, int fontWeight,
                                      NeoWPF::UI::FontStyle fontStyle, int& outWidth,
                                      int& outHeight, int& outInkTop, int& outInkHeight)
{
    const auto bitmap = Rasterize(text, fontSize, fontFamily, fontWeight, fontStyle);
    outWidth = maxWidth > 0 ? (std::min)(bitmap.width, maxWidth) : bitmap.width;
    outHeight = maxHeight > 0 ? (std::min)(bitmap.height, maxHeight) : bitmap.height;
    outInkTop = bitmap.inkTop;
    outInkHeight = bitmap.inkHeight;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Debug,
        "[TextRenderer] RenderTextBitmap textLength=%zu output=%dx%d inkTop=%d inkHeight=%d rgba=%zu",
        text.size(), outWidth, outHeight, outInkTop, outInkHeight, bitmap.rgba.size());
    if (outWidth == bitmap.width && outHeight == bitmap.height) {
        return bitmap.rgba;
    }
    std::vector<std::uint8_t> clipped(static_cast<std::size_t>(outWidth) * outHeight * 4);
    for (int row = 0; row < outHeight; ++row) {
        std::copy_n(bitmap.rgba.data() + static_cast<std::size_t>(row) * bitmap.stride,
                    static_cast<std::size_t>(outWidth) * 4,
                    clipped.data() + static_cast<std::size_t>(row) * outWidth * 4);
    }
    return clipped;
}

}