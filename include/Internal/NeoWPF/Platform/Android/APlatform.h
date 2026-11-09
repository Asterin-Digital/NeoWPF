/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IPlatform.h>
#include <NeoWPF/Platform/Android/AWindowing.h>
#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>

#include <memory>

namespace NeoWPF::Platform::Android {

/*
    @summary
    Provides Android services and the Vulkan-backed Activity host.

    @internal
*/
class APlatform final : public NeoWPF::Contracts::IPlatform {
public:
    explicit APlatform(std::filesystem::path applicationDataDirectory);
    ~APlatform() override;

    /*
        @summary
        Returns the unsupported Android clipboard service.

        @internal
    */
    NeoWPF::Contracts::IClipboard& Clipboard() noexcept override;
    /*
        @summary
        Returns the Android windowing service.

        @internal
    */
    NeoWPF::Contracts::IWindowing& Windowing() noexcept override;
    /*
        @summary
        Returns the Vulkan rendering service.

        @internal
    */
    NeoWPF::Contracts::IRendering& Rendering() noexcept override;
    /*
        @summary
        Android does not expose desktop window chrome.

        @internal
    */
    NeoWPF::Contracts::IWindowChrome* Chrome() noexcept override;
    /*
        @summary
        Returns the Android platform identifier.

        @internal
    */
    NeoWPF::Platform::Capabilities::PlatformId GetPlatformId() const noexcept override;
    /*
        @summary
        Returns the mobile device category.

        @internal
    */
    NeoWPF::Core::DeviceCategory GetDeviceCategory() const noexcept override;
    /*
        @summary
        Waits for the first surface and enters the shared runtime.

        @internal
    */
    int RunApplication(std::chrono::seconds startupTimeout) override;

    /*
        @summary
        Embedded and APK resource loading is not implemented yet.

        @internal
    */
    bool LoadResourceData(const std::wstring& resourcePath,
                          std::vector<std::uint8_t>& outData) override;
    /*
        @summary
        Android has no executable directory.

        @internal
    */
    std::filesystem::path GetExecutableDirectory() const override;
    /*
        @summary
        Returns the Android Activity's private data directory.

        @internal
    */
    NeoWPF::Result<std::filesystem::path> GetAppDataDirectory() const override;
    /*
        @summary
        HTTP access is not implemented yet.

        @internal
    */
    bool FetchHttpResource(const std::wstring& url,
                           std::vector<std::uint8_t>& outData) override;
    /*
        @summary
        Document picker is not implemented yet.

        @internal
    */
    NeoWPF::Contracts::FileDialogResult ShowOpenFileDialog(
        const std::wstring& title, const std::wstring& initialDirectory,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
        bool allowMultiSelect = false) override;
    /*
        @summary
        Android's asynchronous document picker is not implemented yet.

        @internal
    */
    NeoWPF::Contracts::FileDialogResult ShowSaveFileDialog(
        const std::wstring& title, const std::wstring& initialDirectory,
        const std::wstring& defaultFileName,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters) override;
    /*
        @summary
        Image decoding is not implemented yet.

        @internal
    */
    bool DecodeImageFromMemory(const std::vector<std::uint8_t>& data,
                               std::vector<std::uint8_t>& outPixels,
                               int& outWidth, int& outHeight,
                               int decodeWidth, int decodeHeight) override;
    /*
        @summary
        Image decoding is not implemented yet.

        @internal
    */
    bool DecodeImageFile(const std::wstring& filePath,
                         std::vector<std::uint8_t>& outPixels,
                         int& outWidth, int& outHeight,
                         int decodeWidth, int decodeHeight) override;
    /*
        @summary
        Registers a font file with the shared text renderer.

        @internal
    */
    bool RegisterFontFile(const std::wstring& filePath) override;
    /*
        @summary
        Registers font data with the shared text renderer.

        @internal
    */
    bool RegisterFontMemory(const unsigned char* data, std::size_t size,
                            const std::wstring& familyName) override;
    /*
        @summary
        Resolves font requests using registered fonts and the embedded fallback font.

        @internal
    */
    std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily) override;
    /*
        @summary
        Call-stack capture is not implemented yet.

        @internal
    */
    int CaptureCallStack(NeoWPF::Diagnostics::CallStackEntry* entries,
                         int maxEntries, int skipFrames) noexcept override;
    /*
        @summary
        Crash handling remains a safe no-op for this milestone.

        @internal
    */
    void InstallCrashHandler() noexcept override;
    /*
        @summary
        Crash handling remains a safe no-op for this milestone.

        @internal
    */
    void UninstallCrashHandler() noexcept override;
    /*
        @summary
        Returns the current native module path when available.

        @internal
    */
    std::wstring CurrentModulePath() const override;
    /*
        @summary
        Returns the current Linux-kernel thread identifier.

        @internal
    */
    std::uint32_t CurrentThreadId() const noexcept override;

private:
    std::unique_ptr<NeoWPF::Contracts::IClipboard> m_clipboard;
    std::filesystem::path m_applicationDataDirectory;
    AWindowing m_windowing;
    NeoWPF::Rendering::Vulkan::VulkanRendering m_rendering;
};

}