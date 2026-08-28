#include <NeoWPF/Platform/Linux/LPlatform.h>

#include <NeoWPF/Platform/Linux/LWindow.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Rendering/TextRenderer.h>

#include <fontconfig/fontconfig.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

namespace NeoWPF::Platform::Linux {

namespace {

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

bool LPlatform::LoadResourceData(const std::wstring&, std::vector<std::uint8_t>&) { return false; }
std::filesystem::path LPlatform::GetExecutableDirectory() const { return std::filesystem::current_path(); }
bool LPlatform::FetchHttpResource(const std::wstring&, std::vector<std::uint8_t>&) { return false; }
bool LPlatform::DecodeImageFromMemory(const std::vector<std::uint8_t>&, std::vector<std::uint8_t>&,
                                      int&, int&, int, int) { return false; }
bool LPlatform::DecodeImageFile(const std::wstring&, std::vector<std::uint8_t>&,
                                int&, int&, int, int) { return false; }

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
    LoadSystemFontFamily(requestedFontFamily);
    return NeoWPF::Rendering::Text::ResolveFontFamily(requestedFontFamily);
}

int LPlatform::CaptureCallStack(NeoWPF::Diagnostics::CallStackEntry*, int, int) noexcept { return 0; }
void LPlatform::InstallCrashHandler() noexcept {}
void LPlatform::UninstallCrashHandler() noexcept {}
std::wstring LPlatform::CurrentModulePath() const { return {}; }
std::uint32_t LPlatform::CurrentThreadId() const noexcept { return static_cast<std::uint32_t>(getpid()); }

}
