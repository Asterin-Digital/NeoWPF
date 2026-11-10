#include <NeoWPF/Platform/Linux/LPlatform.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Resources/Embedded/FallbackFont.h>
#include <NeoWPF/Platform/Linux/LWindow.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Rendering/TextRenderer.h>

#include <fontconfig/fontconfig.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cwctype>
#include <mutex>
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

std::filesystem::path AbsoluteEnvironmentPath(const char* name)
{
    static std::mutex environmentMutex;
    std::string valueCopy;
    {
        std::lock_guard<std::mutex> lock(environmentMutex);
        const char* value = std::getenv(name);
        if (!value) {
            return {};
        }
        valueCopy.assign(value);
    }
    if (valueCopy.empty()) {
        return {};
    }

    std::filesystem::path path(valueCopy);
    return path.is_absolute() ? path : std::filesystem::path{};
}

std::filesystem::path ResolveApplicationDataDirectory()
{
    auto directory = AbsoluteEnvironmentPath("XDG_DATA_HOME");
    if (!directory.empty()) {
        return directory;
    }

    const auto homeDirectory = AbsoluteEnvironmentPath("HOME");
    if (homeDirectory.empty()) {
        return {};
    }

    return homeDirectory / ".local" / "share";
}

bool LoadEmbeddedFallbackFont()
{
    using NeoWPF::Core::ApplicationResources::Embedded::FontRecord;
    using NeoWPF::Core::ApplicationResources::Embedded::GetEmbeddedFallbackFontCount;
    using NeoWPF::Core::ApplicationResources::Embedded::TryGetEmbeddedFallbackFont;

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
        : m_applicationDataDirectory(ResolveApplicationDataDirectory()),
            m_clipboard(m_windowing)
{
}

LPlatform::~LPlatform() = default;

NeoWPF::Contracts::IClipboard& LPlatform::Clipboard() noexcept { return m_clipboard; }
NeoWPF::Contracts::IWindowing& LPlatform::Windowing() noexcept { return m_windowing; }
NeoWPF::Contracts::IRendering& LPlatform::Rendering() noexcept { return m_rendering; }
NeoWPF::Contracts::IWindowChrome* LPlatform::Chrome() noexcept { return &m_chrome; }
NeoWPF::Platform::Capabilities::PlatformId LPlatform::GetPlatformId() const noexcept
{
    return NeoWPF::Platform::Capabilities::PlatformId::Linux;
}

NeoWPF::Core::DeviceCategory LPlatform::GetDeviceCategory() const noexcept
{
    return NeoWPF::Core::DeviceCategory::Desktop;
}
int LPlatform::RunApplication(std::chrono::seconds startupTimeout)
{
    return NeoWPF::Core::RunDesktopApp(startupTimeout);
}

NeoWPF::Result<std::vector<std::uint8_t>> LPlatform::LoadResourceData(const std::wstring&)
{
    return NeoWPF::Fail<std::string>{"Linux embedded resource loading is unavailable"};
}
std::filesystem::path LPlatform::GetExecutableDirectory() const { return std::filesystem::current_path(); }
NeoWPF::Result<std::filesystem::path> LPlatform::GetAppDataDirectory() const
{
    if (m_applicationDataDirectory.empty()) {
        return NeoWPF::Fail<std::string>{"XDG_DATA_HOME and HOME do not provide a valid absolute data directory."};
    }

    return NeoWPF::Success<std::filesystem::path>{m_applicationDataDirectory};
}

NeoWPF::Result<std::vector<std::uint8_t>> LPlatform::FetchHttpResource(const std::wstring& url)
{
    return m_http.Fetch(url);
}

NeoWPF::Contracts::FileDialogResult LPlatform::ShowOpenFileDialog(
    const std::wstring& title,
    const std::wstring& initialDirectory,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
    bool allowMultiSelect)
{
    return NeoWPF::Platform::Linux::LFileDialog::ShowOpenFileDialog(title, initialDirectory, filters, allowMultiSelect);
}

NeoWPF::Contracts::FileDialogResult LPlatform::ShowSaveFileDialog(
    const std::wstring& title,
    const std::wstring& initialDirectory,
    const std::wstring& defaultFileName,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters)
{
    return NeoWPF::Platform::Linux::LFileDialog::ShowSaveFileDialog(title, initialDirectory, defaultFileName, filters);
}

NeoWPF::Result<NeoWPF::Contracts::DecodedImage> LPlatform::DecodeImageFromMemory(
    const std::vector<std::uint8_t>& data, int decodeWidth, int decodeHeight)
{
    NeoWPF::Contracts::DecodedImage image;
    if (!m_imageDecoding.DecodeImageFromMemory(data, image.Pixels, image.Width, image.Height,
                                               decodeWidth, decodeHeight)) {
        return NeoWPF::Fail<std::string>{"Image data could not be decoded"};
    }
    return NeoWPF::Success<NeoWPF::Contracts::DecodedImage>{std::move(image)};
}

NeoWPF::Result<NeoWPF::Contracts::DecodedImage> LPlatform::DecodeImageFile(
    const std::wstring& filePath, int decodeWidth, int decodeHeight)
{
    NeoWPF::Contracts::DecodedImage image;
    if (!m_imageDecoding.DecodeImageFile(filePath, image.Pixels, image.Width, image.Height,
                                         decodeWidth, decodeHeight)) {
        return NeoWPF::Fail<std::string>{"Image file could not be decoded"};
    }
    return NeoWPF::Success<NeoWPF::Contracts::DecodedImage>{std::move(image)};
}

NeoWPF::Result<void> LPlatform::RegisterFontFile(const std::wstring& filePath)
{
    if (!NeoWPF::Rendering::Text::RegisterFontFile(filePath)) {
        return NeoWPF::Fail<std::string>{"Font file could not be registered"};
    }
    return NeoWPF::Success<void>{};
}

NeoWPF::Result<void> LPlatform::RegisterFontMemory(
    const unsigned char* data, std::size_t size, const std::wstring& familyName)
{
    if (!NeoWPF::Rendering::Text::RegisterFontMemory(data, size, familyName)) {
        return NeoWPF::Fail<std::string>{"Font data could not be registered"};
    }
    return NeoWPF::Success<void>{};
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
        using NeoWPF::Core::ApplicationResources::Embedded::FontRecord;
        using NeoWPF::Core::ApplicationResources::Embedded::GetEmbeddedFallbackFontCount;
        using NeoWPF::Core::ApplicationResources::Embedded::TryGetEmbeddedFallbackFont;

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
