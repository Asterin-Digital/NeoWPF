#include <NeoWPF/Platform/Linux/LPlatform.h>

#include <NeoWPF/Core/Application/Resources/Embedded/FallbackFont.h>
#include <NeoWPF/Platform/Linux/LWindow.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Rendering/TextRenderer.h>

#include <fontconfig/fontconfig.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <cwctype>
#include <unistd.h>

namespace NeoWPF::Platform::Linux {

int CaptureCallStackLinux(NeoWPF::Diagnostics::CallStackEntry* entries,
                          int maxEntries, int skipFrames) noexcept;
void InstallCrashHandlerLinux() noexcept;
void UninstallCrashHandlerLinux() noexcept;
std::wstring CurrentModulePathLinux();
std::uint32_t CurrentThreadIdLinux() noexcept;

namespace {

std::wstring NormalizeFamily(std::wstring family)
{
    std::wstring normalized;
    normalized.reserve(family.size());
    for (const auto character : family) {
        if (!std::iswspace(character)) {
            normalized.push_back(static_cast<wchar_t>(std::towlower(character)));
        }
    }
    return normalized;
}

bool LoadEmbeddedFallbackFont()
{
    using NeoWPF::Core::Application::Resources::Embedded::FontRecord;
    using NeoWPF::Core::Application::Resources::Embedded::GetEmbeddedFallbackFontCount;
    using NeoWPF::Core::Application::Resources::Embedded::TryGetEmbeddedFallbackFont;

    for (std::size_t index = 0; index < GetEmbeddedFallbackFontCount(); ++index) {
        FontRecord record{};
        if (!TryGetEmbeddedFallbackFont(index, record) || !record.data || record.size == 0) {
            continue;
        }

        const auto familyName = NeoWPF::Platform::Utf8ToWide(record.name ? record.name : "Noto Sans");
        if (NeoWPF::Rendering::Text::RegisterFontMemory(record.data, record.size, familyName)) {
            return true;
        }
    }
    return false;
}

bool LoadSystemFontFamily(const std::wstring& requestedFamily)
{
    if (requestedFamily.empty() || !FcInit()) {
        return false;
    }

    FcConfig* config = FcInitLoadConfigAndFonts();
    FcPattern* pattern = FcPatternCreate();
    if (!config || !pattern) {
        if (pattern) FcPatternDestroy(pattern);
        if (config) FcConfigDestroy(config);
        return false;
    }

    const auto familyUtf8 = NeoWPF::Platform::WideToUtf8(requestedFamily);
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(familyUtf8.c_str()));
    FcConfigSubstitute(config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result = FcResultNoMatch;
    FcPattern* match = FcFontMatch(config, pattern, &result);
    bool loaded = false;
    if (match && result == FcResultMatch) {
        FcChar8* file = nullptr;
        if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch && file) {
            loaded = NeoWPF::Rendering::Text::RegisterFontFile(
                NeoWPF::Platform::Utf8ToWide(reinterpret_cast<const char*>(file)));
        }
    }
    if (match) FcPatternDestroy(match);
    FcPatternDestroy(pattern);
    FcConfigDestroy(config);
    return loaded;

}

}

LPlatform::LPlatform()
    = default;

LPlatform::~LPlatform() = default;

const char* LPlatform::Name() const noexcept { return "Linux"; }
NeoWPF::Contracts::IWindowing& LPlatform::Windowing() noexcept { return m_windowing; }
NeoWPF::Contracts::IRendering& LPlatform::Rendering() noexcept { return m_rendering; }
NeoWPF::Contracts::IWindowChrome* LPlatform::Chrome() noexcept { return &m_chrome; }

bool LPlatform::LoadResourceData(const std::wstring&, std::vector<std::uint8_t>&) { return false; }
std::filesystem::path LPlatform::GetExecutableDirectory() const { return std::filesystem::current_path(); }
bool LPlatform::FetchHttpResource(const std::wstring& url, std::vector<std::uint8_t>& outData)
{
    return m_http.Fetch(url, outData);
}

bool LPlatform::DecodeImageFromMemory(const std::vector<std::uint8_t>& data,
                                      std::vector<std::uint8_t>& outPixels,
                                      int& outWidth, int& outHeight,
                                      int decodeWidth, int decodeHeight)
{
    return m_imageDecoding.DecodeImageFromMemory(data, outPixels, outWidth, outHeight,
                                                 decodeWidth, decodeHeight);
}

bool LPlatform::DecodeImageFile(const std::wstring& filePath,
                                std::vector<std::uint8_t>& outPixels,
                                int& outWidth, int& outHeight,
                                int decodeWidth, int decodeHeight)
{
    return m_imageDecoding.DecodeImageFile(filePath, outPixels, outWidth, outHeight,
                                           decodeWidth, decodeHeight);
}

bool LPlatform::RegisterFontFile(const std::wstring& filePath)
{
    return NeoWPF::Rendering::Text::RegisterFontFile(filePath);
}

bool LPlatform::RegisterFontMemory(const unsigned char* data, std::size_t size, const std::wstring& familyName)
{
    return NeoWPF::Rendering::Text::RegisterFontMemory(data, size, familyName);
}

std::wstring LPlatform::ResolveFontFamily(const std::wstring& requestedFontFamily)
{
    if (!requestedFontFamily.empty()) {
        LoadSystemFontFamily(requestedFontFamily);
        const auto resolved = NeoWPF::Rendering::Text::ResolveFontFamily(requestedFontFamily);
        if (!resolved.empty()) {
            return resolved;
        }
    }

    if (LoadEmbeddedFallbackFont()) {
        using NeoWPF::Core::Application::Resources::Embedded::FontRecord;
        using NeoWPF::Core::Application::Resources::Embedded::GetEmbeddedFallbackFontCount;
        using NeoWPF::Core::Application::Resources::Embedded::TryGetEmbeddedFallbackFont;

        for (std::size_t index = 0; index < GetEmbeddedFallbackFontCount(); ++index) {
            FontRecord record{};
            if (!TryGetEmbeddedFallbackFont(index, record) || !record.name || !record.data || record.size == 0) {
                continue;
            }

            const auto resolved = NeoWPF::Rendering::Text::ResolveFontFamily(NeoWPF::Platform::Utf8ToWide(record.name));
            if (!resolved.empty()) {
                return resolved;
            }
        }
    }

    return NeoWPF::Rendering::Text::ResolveFontFamily(L"Sans");
}

int LPlatform::CaptureCallStack(NeoWPF::Diagnostics::CallStackEntry* entries,
                                int maxEntries, int skipFrames) noexcept
{
    return CaptureCallStackLinux(entries, maxEntries, skipFrames);
}

void LPlatform::InstallCrashHandler() noexcept
{
    InstallCrashHandlerLinux();
}

void LPlatform::UninstallCrashHandler() noexcept
{
    UninstallCrashHandlerLinux();
}

std::wstring LPlatform::CurrentModulePath() const
{
    return CurrentModulePathLinux();
}

std::uint32_t LPlatform::CurrentThreadId() const noexcept
{
    return CurrentThreadIdLinux();
}

}
