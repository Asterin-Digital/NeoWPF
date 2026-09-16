#include <NeoWPF/Platform/Windows/WPlatform.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Core/Application/Resources/Embedded/FallbackFont.h>
#include <NeoWPF/Diagnostics/CallStackTracer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <windows.h>
#include <shlobj.h>

#include <filesystem>
#include <cwctype>

namespace NeoWPF::Platform {

bool LoadResourceData(const std::wstring& resourcePath, std::vector<std::uint8_t>& outData);
std::filesystem::path GetExecutableDirectory();
bool DecodeImageFromMemory(const std::vector<std::uint8_t>& data, std::vector<std::uint8_t>& outPixels,
                           int& outWidth, int& outHeight, int decodeWidth, int decodeHeight);
bool DecodeImageFile(const std::wstring& filePath, std::vector<std::uint8_t>& outPixels,
                     int& outWidth, int& outHeight, int decodeWidth, int decodeHeight);

int CaptureCallStackWindows(NeoWPF::Diagnostics::CallStackEntry* entries,
                            int maxEntries, int skipFrames) noexcept;
void InstallCrashHandlerWindows() noexcept;
void UninstallCrashHandlerWindows() noexcept;
std::wstring CurrentModulePathWindows();
std::uint32_t CurrentThreadIdWindows() noexcept;

}

namespace NeoWPF::Platform::Windows {
namespace {

std::filesystem::path FindFontsDirectory()
{
    PWSTR knownFolder = nullptr;
    if (FAILED(::SHGetKnownFolderPath(FOLDERID_Fonts, KF_FLAG_DEFAULT, nullptr, &knownFolder))) {
        return {};
    }

    std::filesystem::path fontsDirectory(knownFolder);
    ::CoTaskMemFree(knownFolder);

    return fontsDirectory;
}

std::filesystem::path FindDefaultFont()
{
    const auto fontsDirectory = FindFontsDirectory();
    if (fontsDirectory.empty()) {
        return {};
    }
    const auto defaultFont = fontsDirectory / L"segoeui.ttf";
    return std::filesystem::is_regular_file(defaultFont) ? defaultFont : std::filesystem::path{};
}

std::wstring NormalizeFamily(std::wstring family)
{
    std::wstring normalized;
    for (const auto character : family) {
        if (!std::iswspace(character)) {
            normalized.push_back(static_cast<wchar_t>(std::towlower(character)));
        }
    }
    return normalized;
}

bool LoadEmbeddedFallbackFont()
{
    using NeoWPF::Core::ApplicationResources::Embedded::FontRecord;
    using NeoWPF::Core::ApplicationResources::Embedded::TryGetEmbeddedFallbackFont;
    using NeoWPF::Core::ApplicationResources::Embedded::GetEmbeddedFallbackFontCount;

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
    HKEY key = nullptr;
    if (RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
            0, KEY_READ, &key) != ERROR_SUCCESS) {
        return false;
    }

    const auto fontsDirectory = FindFontsDirectory();
    const auto normalizedRequested = NormalizeFamily(requestedFamily);
    bool loaded = false;

    for (DWORD index = 0; !loaded; ++index) {
        wchar_t valueName[512] = {};
        wchar_t fileName[512] = {};
        DWORD valueNameSize = static_cast<DWORD>(std::size(valueName));
        DWORD fileNameSize = static_cast<DWORD>(sizeof(fileName));
        DWORD type = 0;
        const auto result = RegEnumValueW(
            key, index, valueName, &valueNameSize, nullptr, &type,
            reinterpret_cast<LPBYTE>(fileName), &fileNameSize);
        if (result != ERROR_SUCCESS) {
            break;
        }
        if (type != REG_SZ && type != REG_EXPAND_SZ) {
            continue;
        }

        std::wstring familyName(valueName, valueNameSize);
        const auto styleStart = familyName.find(L'(');
        if (styleStart != std::wstring::npos) {
            familyName.resize(styleStart);
        }
        while (!familyName.empty() && std::iswspace(familyName.back())) {
            familyName.pop_back();
        }
        if (NormalizeFamily(familyName) != normalizedRequested) {
            continue;
        }

        std::wstring resolvedFileName(fileName, fileNameSize / sizeof(wchar_t));
        if (type == REG_EXPAND_SZ) {
            wchar_t expandedName[512] = {};
            const auto expandedSize = ExpandEnvironmentStringsW(
                resolvedFileName.c_str(), expandedName, static_cast<DWORD>(std::size(expandedName)));
            if (expandedSize > 0 && expandedSize <= std::size(expandedName)) {
                resolvedFileName.assign(expandedName, expandedSize - 1);
            }
        }

        std::filesystem::path fontPath(resolvedFileName);
        if (fontPath.is_relative() && !fontsDirectory.empty()) {
            fontPath = fontsDirectory / fontPath;
        }
        if (std::filesystem::is_regular_file(fontPath)) {
            loaded = NeoWPF::Rendering::Text::RegisterFontFile(fontPath.wstring());
        }
    }

    RegCloseKey(key);
    return loaded;
}

}

WPlatform::WPlatform()
{
    const auto defaultFont = FindDefaultFont();
    if (!defaultFont.empty()) {
        RegisterFontFile(defaultFont.wstring());
    }
}

const char* WPlatform::Name() const noexcept
{
    return "Windows";
}

NeoWPF::Contracts::IWindowing& WPlatform::Windowing() noexcept
{
    return m_windowing;
}

NeoWPF::Contracts::IRendering& WPlatform::Rendering() noexcept
{
    return m_rendering;
}

NeoWPF::Contracts::IWindowChrome* WPlatform::Chrome() noexcept
{
    return &m_chrome;
}

bool WPlatform::LoadResourceData(const std::wstring& resourcePath, std::vector<std::uint8_t>& outData)
{
    return NeoWPF::Platform::LoadResourceData(resourcePath, outData);
}

std::filesystem::path WPlatform::GetExecutableDirectory() const
{
    return NeoWPF::Platform::GetExecutableDirectory();
}

bool WPlatform::FetchHttpResource(const std::wstring& url, std::vector<std::uint8_t>& outData)
{
    return m_http.Fetch(url, outData);
}

NeoWPF::Contracts::FileDialogResult WPlatform::ShowOpenFileDialog(
    const std::wstring& title,
    const std::wstring& initialDirectory,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
    bool allowMultiSelect)
{
    return NeoWPF::Platform::Windows::WFileDialog::ShowOpenFileDialog(
        title, initialDirectory, filters, allowMultiSelect);
}

NeoWPF::Contracts::FileDialogResult WPlatform::ShowSaveFileDialog(
    const std::wstring& title,
    const std::wstring& initialDirectory,
    const std::wstring& defaultFileName,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters)
{
    return NeoWPF::Platform::Windows::WFileDialog::ShowSaveFileDialog(
        title, initialDirectory, defaultFileName, filters);
}

bool WPlatform::DecodeImageFromMemory(const std::vector<std::uint8_t>& data, std::vector<std::uint8_t>& outPixels,
                                      int& outWidth, int& outHeight, int decodeWidth, int decodeHeight)
{
    return NeoWPF::Platform::DecodeImageFromMemory(data, outPixels, outWidth, outHeight, decodeWidth, decodeHeight);
}

bool WPlatform::DecodeImageFile(const std::wstring& filePath, std::vector<std::uint8_t>& outPixels,
                                int& outWidth, int& outHeight, int decodeWidth, int decodeHeight)
{
    return NeoWPF::Platform::DecodeImageFile(filePath, outPixels, outWidth, outHeight, decodeWidth, decodeHeight);
}

bool WPlatform::RegisterFontFile(const std::wstring& filePath)
{
    if (filePath.empty() || ::AddFontResourceExW(filePath.c_str(), FR_PRIVATE, nullptr) <= 0) {
        return false;
    }
    return NeoWPF::Rendering::Text::RegisterFontFile(filePath);
}

bool WPlatform::RegisterFontMemory(const unsigned char* data, std::size_t size, const std::wstring& familyName)
{
    return NeoWPF::Rendering::Text::RegisterFontMemory(data, size, familyName);
}

std::wstring WPlatform::ResolveFontFamily(const std::wstring& requestedFontFamily)
{
    if (!requestedFontFamily.empty()) {
        LoadSystemFontFamily(requestedFontFamily);
        const auto resolved = NeoWPF::Rendering::Text::ResolveFontFamily(requestedFontFamily);
        if (!resolved.empty()) {
            return resolved;
        }
    }

    if (LoadEmbeddedFallbackFont()) {
        using NeoWPF::Core::ApplicationResources::Embedded::GetEmbeddedFallbackFontCount;
        using NeoWPF::Core::ApplicationResources::Embedded::TryGetEmbeddedFallbackFont;
        using NeoWPF::Core::ApplicationResources::Embedded::FontRecord;

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

int WPlatform::CaptureCallStack(NeoWPF::Diagnostics::CallStackEntry* entries,
                                int maxEntries, int skipFrames) noexcept
{
    return CaptureCallStackWindows(entries, maxEntries, skipFrames);
}

void WPlatform::InstallCrashHandler() noexcept
{
    InstallCrashHandlerWindows();
}

void WPlatform::UninstallCrashHandler() noexcept
{
    UninstallCrashHandlerWindows();
}

std::wstring WPlatform::CurrentModulePath() const
{
    return CurrentModulePathWindows();
}

std::uint32_t WPlatform::CurrentThreadId() const noexcept
{
    return CurrentThreadIdWindows();
}

}
