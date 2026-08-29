#pragma once

#include <NeoWPF/Contracts/IPlatform.h>
#include <NeoWPF/Platform/CurlHttp.h>
#include <NeoWPF/Platform/Windows/WWindowing.h>
#include <NeoWPF/Rendering/D3D11/WRendering.h>

namespace NeoWPF::Platform::Windows {

class WPlatform final : public NeoWPF::Contracts::IPlatform {
public:
    WPlatform();

    const char* Name() const noexcept override;
    NeoWPF::Contracts::IWindowing& Windowing() noexcept override;
    NeoWPF::Contracts::IRendering& Rendering() noexcept override;

    bool LoadResourceData(const std::wstring& resourcePath, std::vector<std::uint8_t>& outData) override;
    std::filesystem::path GetExecutableDirectory() const override;
    bool FetchHttpResource(const std::wstring& url, std::vector<std::uint8_t>& outData) override;
    bool DecodeImageFromMemory(const std::vector<std::uint8_t>& data, std::vector<std::uint8_t>& outPixels,
                               int& outWidth, int& outHeight, int decodeWidth, int decodeHeight) override;
    bool DecodeImageFile(const std::wstring& filePath, std::vector<std::uint8_t>& outPixels,
                         int& outWidth, int& outHeight, int decodeWidth, int decodeHeight) override;
    bool RegisterFontFile(const std::wstring& filePath) override;
    bool RegisterFontMemory(const unsigned char* data, std::size_t size, const std::wstring& familyName) override;
    std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily) override;
    int CaptureCallStack(NeoWPF::Diagnostics::CallStackEntry* entries,
                         int maxEntries, int skipFrames) noexcept override;
    void InstallCrashHandler() noexcept override;
    void UninstallCrashHandler() noexcept override;
    std::wstring CurrentModulePath() const override;
    std::uint32_t CurrentThreadId() const noexcept override;

private:
    NeoWPF::Platform::CurlHttp m_http;
    WWindowing m_windowing;
    NeoWPF::Rendering::D3D11::WRendering m_rendering;
};

}
