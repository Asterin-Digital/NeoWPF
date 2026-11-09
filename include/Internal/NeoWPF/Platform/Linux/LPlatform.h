/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IPlatform.h>
#include <NeoWPF/Platform/CurlHttp.h>
#include <NeoWPF/Platform/Linux/LClipboard.h>
#include <NeoWPF/Platform/Linux/LFileDialog.h>
#include <NeoWPF/Platform/Linux/LImageDecoding.h>
#include <NeoWPF/Platform/Linux/LWindowChrome.h>
#include <NeoWPF/Platform/Linux/LWindowing.h>
#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>

namespace NeoWPF::Platform::Linux {

class LPlatform final : public NeoWPF::Contracts::IPlatform {
public:
    LPlatform();
    ~LPlatform() override;

    NeoWPF::Contracts::IClipboard& Clipboard() noexcept override;
    NeoWPF::Contracts::IWindowing& Windowing() noexcept override;
    NeoWPF::Contracts::IRendering& Rendering() noexcept override;
    NeoWPF::Contracts::IWindowChrome* Chrome() noexcept override;
    /*
        @summary
        Returns the Linux platform identifier.

        @internal
    */
    NeoWPF::Platform::Capabilities::PlatformId GetPlatformId() const noexcept override;
    NeoWPF::Core::DeviceCategory GetDeviceCategory() const noexcept override;
    int RunApplication(std::chrono::seconds startupTimeout) override;
    bool LoadResourceData(const std::wstring& resourcePath, std::vector<std::uint8_t>& outData) override;
    std::filesystem::path GetExecutableDirectory() const override;
    /*
        @summary
        Returns the Linux per-user data directory.

        @internal
    */
    NeoWPF::Result<std::filesystem::path> GetAppDataDirectory() const override;
    bool FetchHttpResource(const std::wstring& url, std::vector<std::uint8_t>& outData) override;
    NeoWPF::Contracts::FileDialogResult ShowOpenFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
        bool allowMultiSelect = false) override;
    NeoWPF::Contracts::FileDialogResult ShowSaveFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::wstring& defaultFileName,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters) override;
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
    std::filesystem::path m_applicationDataDirectory;
    NeoWPF::Platform::CurlHttp m_http;
    LImageDecoding m_imageDecoding;
    LWindowing m_windowing;
    LClipboard m_clipboard;
    LWindowChrome m_chrome;
    NeoWPF::Rendering::Vulkan::VulkanRendering m_rendering;
};

}
