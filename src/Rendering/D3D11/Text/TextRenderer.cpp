#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Core/Application/ApplicationResources.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <Windows.h>
#include <dwrite.h>
#include <wrl.h>
#include <any>
#include <algorithm>
#include <cwctype>
#include <mutex>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace NeoWPF::Rendering::Text {

namespace {

std::wstring ResolveRegisteredFontFamily(const std::wstring& requestedFontFamily)
{
    if (requestedFontFamily.empty()) {
        return std::wstring();
    }

    const auto registeredFiles = GetRegisteredFontFiles();
    if (registeredFiles.empty()) {
        return requestedFontFamily;
    }

    for (const auto& filePath : registeredFiles) {
        const std::wstring fileName = filePath.substr(filePath.find_last_of(L"\\/") + 1);
        std::wstring fileNameNoExt = fileName;
        if (const auto dot = fileNameNoExt.find_last_of(L'.'); dot != std::wstring::npos) {
            fileNameNoExt.resize(dot);
        }

        if (requestedFontFamily.size() == fileNameNoExt.size()) {
            bool sameFamily = true;
            for (std::size_t i = 0; i < requestedFontFamily.size(); ++i) {
                if (static_cast<wchar_t>(::towlower(static_cast<wint_t>(requestedFontFamily[i]))) != static_cast<wchar_t>(::towlower(static_cast<wint_t>(fileNameNoExt[i])))) {
                    sameFamily = false;
                    break;
                }
            }
            if (sameFamily) {
                    ::NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Debug, "[TextRenderer] Resolved font family '%ls' to '%ls' from file path '%ls'", requestedFontFamily.c_str(), fileNameNoExt.c_str(), filePath.c_str());
                return fileNameNoExt;
            }
        }
    }

    return requestedFontFamily;
}

}

std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily)
{
    if (!requestedFontFamily.empty()) {
        const std::wstring resolvedFamily = ResolveRegisteredFontFamily(requestedFontFamily);
        if (!resolvedFamily.empty()) {
            return resolvedFamily;
        }
        return requestedFontFamily;
    }

    std::any resourceValue;
    if (NeoWPF::Core::ApplicationResources::Resources::TryGetResource("TextBox.FontFamily", resourceValue) ||
        NeoWPF::Core::ApplicationResources::Resources::TryGetResource("TextBlock.FontFamily", resourceValue) ||
        NeoWPF::Core::ApplicationResources::Resources::TryGetResource("FontFamily", resourceValue)) {
        if (const auto* fontFamily = std::any_cast<std::wstring>(&resourceValue)) {
            const std::wstring resolvedFamily = ResolveRegisteredFontFamily(*fontFamily);
            if (!resolvedFamily.empty()) {
                return resolvedFamily;
            }
            return *fontFamily;
        } else if (const auto* fontFamily = std::any_cast<std::string>(&resourceValue)) {
            const std::wstring resolvedFamily = ResolveRegisteredFontFamily(std::wstring(fontFamily->begin(), fontFamily->end()));
            if (!resolvedFamily.empty()) {
                return resolvedFamily;
            }
            return std::wstring(fontFamily->begin(), fontFamily->end());
        }
    }

    return std::wstring(L"Segoe UI");
}

 

TextMetrics TextMeasureTextInternal(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int maxWidth, int maxHeight);
std::vector<uint8_t> TextRenderTextBitmapInternal(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int& outWidth, int& outHeight, int* outInkTop = nullptr, int* outInkHeight = nullptr);

TextMetrics MeasureText(const std::wstring& text, int fontSize, int maxWidth, int maxHeight)
{
    return TextMeasureTextInternal(text, fontSize, std::wstring(L"Segoe UI"), maxWidth, maxHeight);
}

TextMetrics MeasureText(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int maxWidth, int maxHeight)
{
    return TextMeasureTextInternal(text, fontSize, fontFamily, maxWidth, maxHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int& outWidth, int& outHeight)
{
    return TextRenderTextBitmapInternal(text, fontSize, 0, 0, std::wstring(L"Segoe UI"), outWidth, outHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, int& outWidth, int& outHeight)
{
    return TextRenderTextBitmapInternal(text, fontSize, maxWidth, maxHeight, std::wstring(L"Segoe UI"), outWidth, outHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int& outWidth, int& outHeight)
{
    return TextRenderTextBitmapInternal(text, fontSize, 0, 0, fontFamily, outWidth, outHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int& outWidth, int& outHeight)
{
    return TextRenderTextBitmapInternal(text, fontSize, maxWidth, maxHeight, fontFamily, outWidth, outHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int& outWidth, int& outHeight, int& outInkTop, int& outInkHeight)
{
    outInkTop = 0;
    outInkHeight = 0;
    return TextRenderTextBitmapInternal(text, fontSize, maxWidth, maxHeight, fontFamily, outWidth, outHeight, &outInkTop, &outInkHeight);
}

}