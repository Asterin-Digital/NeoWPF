#include <NeoWPF/Rendering/TextRenderer.h>

#include <NeoWPF/Rendering/Text/FontDatabase.h>
#include <NeoWPF/Rendering/Text/TextRasterizer.h>
#include <NeoWPF/Rendering/Text/TextShaper.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <algorithm>
#include <filesystem>
#include <mutex>
#include <unordered_set>

namespace NeoWPF::Rendering::Text {
namespace {

using NeoWPF::Platform::Utf8ToWide;
using NeoWPF::Platform::WideToUtf8;

FontRequest MakeRequest(const std::wstring& family, int weight, NeoWPF::UI::FontStyle style)
{
    std::wstring effectiveFamily = family;
    if (effectiveFamily.empty()) {
        std::any applicationFamily;
        if (NeoWPF::Core::Application::TryFindResource("FontFamily", applicationFamily)) {
            try {
                effectiveFamily = std::any_cast<std::wstring>(applicationFamily);
            } catch (const std::bad_any_cast&) {
                effectiveFamily.clear();
            }
        }
    }
    if (effectiveFamily.empty()) {
        effectiveFamily = L"Sans";
    }

    static std::mutex platformResolutionMutex;
    static std::unordered_set<std::wstring> platformResolutionAttempts;
    if (!effectiveFamily.empty()) {
        std::lock_guard<std::mutex> lock(platformResolutionMutex);
        if (platformResolutionAttempts.insert(effectiveFamily).second) {
            if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
                platform->ResolveFontFamily(effectiveFamily);
            }
        }
    }

    FontRequest request{WideToUtf8(effectiveFamily), weight > 0 ? weight : 400, style};
    auto record = GetFontDatabase().Resolve(request);
    if (record) {
        request.familyUtf8 = record->familyUtf8;
    }

    static std::once_flag resolutionLogOnce;
    std::call_once(resolutionLogOnce, [&] {
        NeoWPF::Diagnostics::Log(
            NeoWPF::Diagnostics::Category::TextRendering,
            NeoWPF::Diagnostics::Level::Info,
            "[TextRenderer] FontFamily requested='%ls' effective='%ls' resolved='%s' records=%zu",
            family.c_str(), effectiveFamily.c_str(),
            record ? record->familyUtf8.c_str() : "<none>",
            GetFontDatabase().Records().size());
    });
    return request;
}

TextBitmap Rasterize(const std::wstring& text, int fontSize, const std::wstring& family,
                    int weight, NeoWPF::UI::FontStyle style)
{
    auto& fonts = GetFontDatabase();
    const auto request = MakeRequest(family, weight, style);
    const auto glyphs = CreateHarfBuzzTextShaper(fonts)->Shape(WideToUtf8(text), request, fontSize);
    auto bitmap = CreateFreeTypeTextRasterizer(fonts)->Rasterize(glyphs, request, fontSize);
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
    const auto resolved = GetFontDatabase().Resolve({
        WideToUtf8(requestedFontFamily), 400, NeoWPF::UI::FontStyle::Normal});
    if (!resolved) {
        return requestedFontFamily;
    }
    return Utf8ToWide(resolved->familyUtf8);
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