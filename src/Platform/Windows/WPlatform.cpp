#include <NeoWPF/Platform/Windows/WPlatform.h>
#include <NeoWPF/Diagnostics/CallStackTracer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <windows.h>
#include <shlobj.h>

#include <filesystem>

namespace NeoWPF::Platform {

bool LoadResourceData(const std::wstring& resourcePath, std::vector<std::uint8_t>& outData);
std::filesystem::path GetExecutableDirectory();
bool FetchHttpResource(const std::wstring& url, std::vector<std::uint8_t>& outData);
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

std::filesystem::path FindDefaultFont()
{
    PWSTR knownFolder = nullptr;
    if (FAILED(::SHGetKnownFolderPath(FOLDERID_Fonts, KF_FLAG_DEFAULT, nullptr, &knownFolder))) {
        return {};
    }

    std::filesystem::path fontsDirectory(knownFolder);
    ::CoTaskMemFree(knownFolder);

    const auto defaultFont = fontsDirectory / L"segoeui.ttf";
    return std::filesystem::is_regular_file(defaultFont) ? defaultFont : std::filesystem::path{};
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
    return NeoWPF::Platform::FetchHttpResource(url, outData);
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
    return NeoWPF::Rendering::Text::ResolveFontFamily(requestedFontFamily);
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
